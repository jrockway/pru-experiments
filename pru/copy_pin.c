#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_iep.h>
/* #include <rsc_types.h> */
#include <pru_rpmsg.h>
#include "resource_table_0.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

/* Host-0 Interrupt sets bit 30 in register R31 */
#define HOST_INT			((uint32_t) 1 << 30)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST			16
#define FROM_ARM_HOST			17

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 30"
#define CHAN_PORT			30

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

/* Buffer for sending data back to Linux. */
uint8_t payload[RPMSG_BUF_SIZE];

typedef enum { WAIT_RISE, WAIT_FALL, SEND_TIME } state_t;

void main(void)
{
  struct pru_rpmsg_transport transport;
  uint16_t src, dst, len;
  volatile uint8_t *status;
  state_t state = WAIT_RISE;
  uint32_t rise, total;

  /* Allow OCP master port access by the PRU so the PRU can read external memories */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  /* Disable counter */
  CT_IEP.TMR_GLB_CFG_bit.CNT_EN = 0;

  /* Clear overflow status register */
  CT_IEP.TMR_GLB_STS_bit.CNT_OVF = 0x1;

  /* Disable compensation */
  CT_IEP.TMR_COMPEN_bit.COMPEN_CNT = 0x0;

  /* Clear the status of all interrupts */
  CT_INTC.SECR0 = 0xFFFFFFFF;
  CT_INTC.SECR1 = 0xFFFFFFFF;

  /* Enable counter */
  CT_IEP.TMR_GLB_CFG = 0x11;

  /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

  /* Make sure the Linux drivers are ready for RPMsg communication */
  status = &resourceTable.rpmsg_vdev.status;

  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

  /* Initialize the RPMsg transport structure */
  pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

  /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);

  /* We have to wait to get a message from the ARM side before doing anything,
     as the PRU does not know the destination address to write to:
     https://e2e.ti.com/support/arm/sitara_arm/f/791/t/543704 */
  while(1) {
    /* Check bit 30 of register R31 to see if the ARM has kicked us. */
    if (__R31 & HOST_INT) {
      /* Clear the event status */
      CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

      /* Receive all available messages, multiple messages can be sent per kick */
      if (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
        break;
      }
    }
  }

  payload[0] = 0;
  payload[1] = 0;
  *(int *)&payload[2] = 0;
  *(int *)&payload[6] = 0;
  pru_rpmsg_send(&transport, dst, src, payload, 10);

  /* Enable counter */
  CT_IEP.TMR_GLB_CFG = 0x11;
  CT_IEP.TMR_CNT = 0x0;
  total = 0;
  rise = 0;
  
  while(1) {
    switch (state) {
    case WAIT_RISE:
      if ((__R31 & 2) >> 1 == 1) {
        total = CT_IEP.TMR_CNT;
        CT_IEP.TMR_CNT = 0x0;
        state = WAIT_FALL;
      }
      break;
    case WAIT_FALL:
      if ((__R31 & 2) == 0) {
        rise = CT_IEP.TMR_CNT;
        state = SEND_TIME;
      }
      break;
    case SEND_TIME:
      payload[0] = 0;
      payload[1] = 0;
      *(int *)&payload[2] = total;
      *(int *)&payload[6] = rise;
      pru_rpmsg_send(&transport, dst, src, &payload, 10);
      state = WAIT_RISE;
      break;
    default:
      break;
    }
  }
}

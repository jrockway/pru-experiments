#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
  /* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  __R30 = 0x0000;
  
  while (1) {
    __R30 = 0x0001 & ((__R31 & 2) >> 1); // copy gpio 1 state to gpio 0
  }
}

#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

volatile register uint32_t __R30;

void main(void)
{
  __R30 = 0x0002;
          
  /* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  while (1) {
    __R30 = 0x0001;
    __delay_cycles(7);
    __R30 = 0x0000;
    __delay_cycles(10);
  }
}

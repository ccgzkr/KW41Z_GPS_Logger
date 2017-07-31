#include "gpio.h"

void BOARD_InitEmuUARTPins(void) {
  CLOCK_EnableClock(kCLOCK_PortB);                           /* Port C Clock Gate Control: Clock enabled */

  const port_pin_config_t portb2_3_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_SlowSlewRate,                                      /* Slow slew rate is configured */
    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
    kPORT_MuxAlt5,                                           /* Pin is configured as TPM1_CH0 */
  };
  PORT_SetPinConfig(PORTC, PIN4_IDX, &portb2_3_config);      /* PORTB2_3 is configured as TPM1_CH0 and TPM1_CH1 */
  
  PORT_SetPinMux(PORTC, PIN6_IDX, kPORT_MuxAlt4);            /* PORTC6 (pin 42) is configured as UART0_RX */
  PORT_SetPinMux(PORTC, PIN7_IDX, kPORT_MuxAlt4);            /* PORTC7 (pin 43) is configured as UART0_TX */
  SIM->SOPT4 = ((SIM->SOPT4 &
    (~(SIM_SOPT4_TPM1CH0SRC_MASK)))                          /* Mask bits to zero which are setting */
      | SIM_SOPT4_TPM1CH0SRC(SOPT4_TPM1CH0SRC_TPM)           /* TPM1 Channel 0 Input Capture Source Select: TPM1_CH0 
signal */
    );
  SIM->SOPT5 = ((SIM->SOPT5 &
    (~(SIM_SOPT5_LPUART0RXSRC_MASK)))                        /* Mask bits to zero which are setting */
      | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX) /* LPUART0 Receive Data Source Select: LPUART_RX pin */
    );
}




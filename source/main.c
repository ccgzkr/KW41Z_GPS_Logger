#include "main.h"

int main(void)
{   
    tpm_config_t tpmInfo;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetTpmClock(1U);

    TPM_GetDefaultConfig(&tpmInfo);
    TPM_Init(EMUART_TPM, &tpmInfo);

    TPM_SetupInputCapture(EMUART_TPM, CH_SW_RX, kTPM_FallingEdge);
    
    TPM_EnableInterrupts(EMUART_TPM, CH_SW_RX);
    TPM_EnableInterrupts(EMUART_TPM, CH_SW_TX);
    EnableIRQ(EMU_UART_IRQ);
    
    TPM_StartTimer(EMUART_TPM, kTPM_SystemClock);

    
    while(1)                                       //idle loop
    {
        if(swu_rx_fifo[0] == '0')
        {
            GPIO_TogglePinsOutput(LED_PORT, 1 << LED_GPIO_PIN);
        }
    }
}



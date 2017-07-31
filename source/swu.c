/************************************************************************/
/*                                                                      */
/* KW41Z Software UART (Tx&Rx)                                          */
/*                                                                      */
/* ==================================================================== */
/*                                                                      */
/* PTB3: SWU_Tx     [MAT0.3]- software UART Tx pin                      */
/* PTB2: SWU_RX     [CAP0.0]- software UART Rx pin                      */
/* PTB0: SWU_Tx     [PTB0]  - GPIO indicator drive                      */
/*                                                                      */
/************************************************************************/

#include "swu.h"

static inline void TPM_StartTimer(TPM_Type *base, tpm_clock_source_t clockSource);

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

void BOARD_InitPins(void) 
{
    CLOCK_EnableClock(kCLOCK_PortB);
    CLOCK_EnableClock(kCLOCK_PortC);
    const port_pin_config_t portb2_3_config = 
    {
        kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
        kPORT_SlowSlewRate,                                      /* Slow slew rate is configured */
        kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
        kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
        kPORT_MuxAlt5,                                           /* Pin is configured as TPM1_CH0/1 */
    };
		
    PORT_SetPinConfig(EMU_UART_PORT, PIN_SW_RX, &portb2_3_config);
    PORT_SetPinConfig(EMU_UART_PORT, PIN_SW_TX, &portb2_3_config);
		
    PORT_SetPinMux(UART_PORT, PIN_UART_RX, kPORT_MuxAlt4);
    PORT_SetPinMux(UART_PORT, PIN_UART_TX, kPORT_MuxAlt4);

    SIM->SOPT5 = ((SIM->SOPT5 &
    (~(SIM_SOPT5_LPUART0RXSRC_MASK)))                        /* Mask bits to zero which are setting */
      | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX) /* LPUART0 Receive Data Source Select: LPUART_RX pin */
    );
    
    PORT_SetPinMux(EMU_UART_PORT, LED_GPIO_PIN, kPORT_MuxAsGpio);
			
    gpio_pin_config_t led_config = 
    {
        kGPIO_DigitalOutput, 
        LED_OFF,
    };
	
    GPIO_PinInit(LED_PORT, PIN_SW_IND, &led_config);

}

//this routine prepares an array off toggle points that will be used to generate 
//a waveform for the currently selected character in the software UART transmission 
//FIFO; at the end this routine starts the transmission intself
void swu_tx(void)
{
    uint8_t bit,i;
    unsigned long int ext_data, delta_edges, mask, reference;
    if(tx_fifo_wr_ind != tx_fifo_rd_ind)            //data to send, proceed
    {
        swu_status |= TX_ACTIVE;                    //sw uart tx is active
        tx_fifo_rd_ind++;                           //update the tx fifo ...
        if(tx_fifo_rd_ind == TXBUFF_LEN)            //read index...
        {
            tx_fifo_rd_ind = 0;                     //...
        }
        ext_data = (unsigned long int) swu_tx_fifo[tx_fifo_rd_ind]; //read the data
        ext_data = 0xFFFFFE00 | (ext_data << 1);    //prepare the pattern
        edge[0] = BIT_LENGTH;                       //at least 1 falling edge...
        cnt_edges = 1;                              //... because of the START bit
        bit = 1;                                    //set the bit counter
        reference = 0x00000000;                     //init ref is 0 (start bit)
        mask = 1 << 1;                              //prepare the mask
        delta_edges = BIT_LENGTH;                   //next edge at least 1 bit away
        while(bit != 10)                            //until all bits are examined
        {
            if((ext_data & mask) == (reference & mask)) //bit equal to the reference?
            {
                delta_edges += BIT_LENGTH;          //bits identical=>update length
            }
            else                                    //bits are not the same:
            {
                edge[cnt_edges] = edge[cnt_edges - 1] + delta_edges;  //store new edge data
                reference = ~reference;             //update the reference
                delta_edges = BIT_LENGTH;           //reset delta_ to 1 bit only
                cnt_edges++;                        //update the edges counter
            }
            mask = mask << 1;                       //update the mask
            bit++;                                  //move on to the next bit
        }
        
        edge[cnt_edges]= edge[cnt_edges - 1] + delta_edges;  //add the stop bit end to the list
        cnt_edges++;                                //update the number of edges
        last_edge_index = cnt_edges - 2;            //calculate the last edge index
        char_end_index = cnt_edges - 1;             //calc. the character end index
       
        edge_index = 0;                             //reset the edge index
        reference = EMUART_TPM->CNT + BIT_LENGTH;         //get the reference from TIMER0
        for(i = 0; i != cnt_edges; i++)             //recalculate toggle points...
        {
            edge[i] = (edge[i] + reference) & 0x000FFFF;     //... an adjust for the timer range
        }                               //... 
        EMUART_TPM->CONTROLS[1].CnV = edge[0];   //load MR3
        EMUART_TPM->CONTROLS[1].CnSC = 0xD4;     //enable interrupt and toggle on MR3 match
    }
    return;                                         //return from the routine
}

//this is the TIMER0 interrupt service routine
//key software UART transmit and receive code resides here
void TPM1_IRQHandler(void)
{
    signed long int edge_temp;
    //sw uart receive isr code begin
    if((EMUART_TPM->STATUS & TPM_STATUS_CH0F(0x01)) != 0x00)      //capture interrupt occured:
    {
        EMUART_TPM->STATUS = TPM_STATUS_CH0F(0x01);               //edge detected=>clear CAP0 flag
        EMUART_TPM->CONTROLS[0].CnSC = 0x00D0 | (0x000C - (EMUART_TPM->CONTROLS[0].CnSC & 0x000C)); //reverse the targeted edge
        if((swu_status & RX_ACTIVE) == 0)           //sw UART not active (start):
        {
            edge_last = (signed long int) EMUART_TPM->CNT;        //initialize the last edge
            edge_sample = edge_last + (BIT_LENGTH >> 1);    //initialize the sample edge, delay bittime/2
            if(edge_sample < edge_last)             //adjust the sample edge...
            {
                edge_sample |= ADJUST;              //... if needed
            }
            swu_bit = 0;                            //rx bit is 0 (a start bit)
            EMUART_TPM->STATUS = 0x01;           //clear MAT1 int flag
            edge_stop = edge_sample + STOP_BIT_SAMPLE;    //estimate the end of the byte
            if(edge_stop < edge_last)               //adjust the end of byte...
            {
                edge_stop |= ADJUST;                //... if needed
            }
            EMUART_TPM->CONTROLS[2].CnV = edge_stop & 0xFFFF;              //set MR1 (stop bit center)
            EMUART_TPM->CONTROLS[2].CnSC = 0xD0; //int on MR1
            cnt = 9;                                //initialize the bit counter
            swu_status |= RX_ACTIVE;                //update the swu status
            swu_rbr = 0x0000;                       //reset the sw rbr
            swu_rbr_mask = 0x0001;                  //initialize the mask
        }
        else                                        //reception in progress:
        {
            edge_current = (signed long int) EMUART_TPM->CNT; //initialize the current edge
            
            if (edge_current < edge_last)           //adjust the current edge...
            {
                edge_current |= ADJUST;             //... if needed
            }
            while(edge_current > edge_sample)       //while sampling edge is within
            {
                if(cnt_bits != 0)
                {
                    if(swu_bit != 0)                //update data...
                    {
                        swu_rbr |= swu_rbr_mask;    //...
                    }
                    swu_rbr_mask = swu_rbr_mask << 1; //update mask
                }
                cnt_bits++;                         //update the bit count
                edge_temp = edge_last + BIT_LENGTH; //estimate the last edge
                if(edge_temp < edge_last)           //adjust...
                {
                    edge_last = edge_temp | ADJUST; //... the last edge...
                }
                else                                //... if...
                {
                    edge_last = edge_temp;          //... needed
                }
                edge_temp = edge_sample + BIT_LENGTH; //estimate the sample edge
                if(edge_temp < edge_sample)           //adjust...
                {
                    edge_sample = edge_temp | ADJUST; //... the sample edge...
                }
                else                                //... if...
                {
                    edge_sample = edge_temp;        //... needed
                }
                cnt--;                              //update the no of rcved bits
            }
            swu_bit = 1 - swu_bit;                  //change the received bit
        }
    }
    if((EMUART_TPM->STATUS & TPM_STATUS_CH2F(0x01)) != 0x00)                       //stop bit timing matched:
    {
        EMUART_TPM->STATUS = TPM_STATUS_CH2F(0x01);                                //clear MR1 flag
        if(cnt != 0)                                //not all data bits received...
        {
            swu_rbr = swu_rbr << cnt;               //... make space for the rest...
            if(swu_bit != 0)
            {
                swu_rbr += ALL1 << (8 - cnt);       //... add needed 1(s)...
            }
        }                                           //...
        swu_rbr &= 0x00FF;                          //extract data bits only
        if(swu_bit == 0)                            //if the stop bit was 0 =>
        {
            swu_rbr |= 0x00000100;                  //... framing error!
        }            
        swu_status &= ~RX_ACTIVE;                   //sw UART not active any more
        cnt_bits = 0;                               //reset the rx bit count
        if(swu_rx_cnt != RXBUFF_LEN)                //store the received character...
        {
            swu_rx_cnt++;                           //... into the sw UART...
            rx_fifo_wr_ind++;                       //... rx FIFO
            if(rx_fifo_wr_ind == RXBUFF_LEN)        //...
            {
                rx_fifo_wr_ind = 0;
            }
            swu_rx_fifo[rx_fifo_wr_ind] = swu_rbr;  //...
            if(swu_rx_cnt >= swu_rx_trigger)
            {
                swu_rx_isr();    //rx 'isr' trig excded
            }
        }
        else
        {
            swu_status |= RX_OVERFLOW;              //rx FIFO full => overflow
        }
        EMUART_TPM->CONTROLS[2].CnSC = (0x10) << 0x06;                       //MR0 impacts TIMER0 no more
    }
    //sw uart receive isr code end
    
    //sw uart transmit isr code begin
    if((EMUART_TPM->STATUS & TPM_STATUS_CH1F(0x01)) != 0x00)      //tx routine interrupt begin
    {
        EMUART_TPM->STATUS = TPM_STATUS_CH1F(0x01);               //clear the MAT flag
        if(edge_index == char_end_index)            //the end of the char:
        {
            EMUART_TPM->CONTROLS[1].CnSC &= ~TPM_CnSC_CHIE(0x01);
            
            swu_tx_cnt--;                           //update no.of chars in tx FIFO
            if(tx_fifo_wr_ind != tx_fifo_rd_ind)    //if more data pending...
            {
                swu_tx();                           //... spin another transmission
            }
            else
            {
                swu_status &= ~TX_ACTIVE;           //no data left=>turn off the tx
            }
        }
        else                                        //not the end of the character:
        {
            if(edge_index == last_edge_index)       //is this the last toggle?
            {
                //T0EMR = 0x000003FF;               //no more toggle on MAT3
                //EMU_UART_PORT->CONTROLS[1].CnSC &= ~TPM_CnSC_CHIE(0x01);
                EMUART_TPM->CONTROLS[1].CnV = 0x00;
            }
            edge_index++;                           //update the edge index
            EMUART_TPM->CONTROLS[1].CnV = edge[edge_index] & 0xFFFF;     //prepare the next toggle event
        }
    }                    //tx routine interrupt end
    //sw uart transmit isr code end

    if(EMUART_TPM->SC & TPM_SC_TOF(0x01) != 0x00)
    {
        EMUART_TPM->SC |= TPM_SC_TOF(0x01);
    }

    //VICVectAddr = 0xFF;                             //update the VIC    
}

void IRQ_default(void)__irq                         //default IRQ isr
{
    //VICVectAddr = 0xFF;                             //update the VIC
}

//this routine transfers a string of characters one by one into
//the software UART tx FIFO
void swu_tx_wr(uint8_t * ptr_out)
{

    while(*ptr_out != 0x00)                         //read all chars...
    {
        swu_tx_wr_chr(*ptr_out);                    //...put the char in tx FIFO...
        ptr_out++;                                  //...move to the next char...
    }                                               //...
    return;                                         //return from the routine
}

//this routine puts a single character into the software UART tx FIFO
void swu_tx_wr_chr(uint8_t out_char)
{
    while(swu_tx_cnt == TXBUFF_LEN);               //wait if the tx FIFO is full
    {
        tx_fifo_wr_ind++;                          //update the write pointer...
    }
    if(tx_fifo_wr_ind == TXBUFF_LEN)               //...
    {
        tx_fifo_wr_ind = 0;                        //...
    }
    swu_tx_fifo[tx_fifo_wr_ind] = out_char;        //put the char into the FIFO
    swu_tx_cnt++;                                  //update no.of chrs in the FIFO
    if((swu_status & TX_ACTIVE) == 0)              //start tx if tx is not active
    {
        swu_tx();
    }
    
    return;                                        //return from the routine
}

//this routine reads a single character from the software UART rx FIFO
//if no new data is available, it returns the last one read; framing error
//indicator is updated, too
uint8_t swu_rx_rd_chr(void)
{
    if(swu_rx_cnt != 0)                             //update the rx indicator...
    {
        rx_fifo_rd_ind++;                           //... if data are present...
        if(rx_fifo_rd_ind == RXBUFF_LEN)   //...
        {
            rx_fifo_rd_ind = 0;
        }
        swu_rx_cnt--;                               //...
    }
    if((swu_rx_fifo[rx_fifo_rd_ind] & 0x0100) == 0) //update...
    {
        swu_rx_chr_fe = 0;                          //... the framing error...
    }        
    else                                            //... indicator...
    {
        swu_rx_chr_fe = 1;                          //...
    }

    swu_status &= ~RX_OVERFLOW;                     //clear the overfloe flag
    return((uint8_t)(swu_rx_fifo[rx_fifo_rd_ind] & 0x00FF));    //return data
}

//this code acts as a standard uart rx interrupt routine for the specified
//received count character trigger; this routine is called at the end
//of the received byte that increased overall number of characters in the 
//rx FIFO to or beyond the specified trigger
void swu_rx_isr(void)
{
    swu_tx_wr_chr(swu_rx_rd_chr());                 //transmit the last rcvd char
    return;                                         //return from the routine
}

uint32_t LPUART0_GetFreq(void)
{
    return CLOCK_GetFreq(kCLOCK_CoreSysClk);
}

#ifndef __SWU_H
#define __SWU_H

/* ==================================================================== */
/*                                                                      */
/* PTB3: SWU_TX     [MAT0.3]- software UART Tx pin                      */
/* PTB2: SWU_RX     [CAP0.0]- software UART Rx pin                      */
/*                                                                      */
/************************************************************************/

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_tpm.h"
#include "fsl_port.h"
#include "board.h"
#include "MKW41Z4.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"

#define EMU_UART_IRQ    (TPM1_IRQn)

#define EMU_UART_PORT   PORTB
#define PIN_SW_RX       ((uint32_t)2)
#define PIN_SW_TX       ((uint32_t)3)
#define PIN_SW_IND      ((uint32_t)0)

#define LED_PORT        GPIOB
#define LED_OFF         ((uint8_t)0)
#define LED_ON          ((uint8_t)1)
#define LED_GPIO_PIN    ((uint32_t)0)

#define EMUART_TPM      TPM1
#define CH_SW_RX        (kTPM_Chnl_0)
#define CH_SW_TX        (kTPM_Chnl_1)

#define UART_PORT       PORTC
#define PIN_UART_RX     ((uint32_t)6)
#define PIN_UART_TX     ((uint32_t)7)

#define UART_RX_SRC     ((uint32_t)0)
#define UART_TX_SRC     ((uint32_t)0)

#define SOPT5_LPUART0RXSRC_LPUART_RX  ((uint32_t)0) 

//#define TPM_COMBINE_SHIFT 						((uint32_t)8)

//void TPM1_IRQHandler(void) __irq;
//void IRQ_default(void) __irq;
volatile uint8_t cnt_edges;                           //no of char edges
volatile uint8_t edge_index;                          //edge index for output
volatile uint8_t swu_tx_st;                           //sw UART status
volatile unsigned long int edge[11];                        //array of edges
volatile uint8_t last_edge_index, char_end_index;     //last two events index

//software UART configurable parameters begin
#define TXBUFF_LEN  ((uint32_t)16)
#define RXBUFF_LEN  ((uint32_t)16)

//40000000/9600 = 4166 PCLKs
//PCLK=40MHz:
#define BIT_LENGTH  ((uint32_t)4166)

//40000000/9600 = 4166 PCLKs
//PCLK=60MHz:
//#define BIT_LENGTH 6250
#define STOP_BIT_SAMPLE (9 * BIT_LENGTH)

//software UART configurable parameters end
//definitions common for transmitting and receivign data begin
volatile unsigned long int swu_status;
#define RX_OVERFLOW (4)
#define RX_ACTIVE   (2)
#define TX_ACTIVE   (1)
#define ADJUST      (1 << 30)
#define ALL1        (0x000000FF)

//definitions common for transmitting and receivign data end
//software UART Tx related definitions begin
volatile unsigned long int tx_fifo_wr_ind,tx_fifo_rd_ind;
volatile signed long int swu_tx_cnt, swu_tx_trigger;
volatile unsigned short int swu_tx_fifo[TXBUFF_LEN];

//software UART Tx related definitions end
//software UART Rx related definitions begin
volatile unsigned long int rx_fifo_wr_ind,rx_fifo_rd_ind;
volatile signed long int swu_rx_cnt, swu_rx_trigger;
volatile uint8_t swu_bit, cnt, cnt_bits, swu_rx_chr_fe;
volatile unsigned long int swu_rbr, swu_rbr_mask;
volatile signed long int edge_last, edge_sample, edge_current, edge_stop;
volatile unsigned short int swu_rx_fifo[RXBUFF_LEN];

void swu_tx(void);              //tx param processing
void swu_tx_wr(uint8_t *);
void swu_tx_wr_chr(uint8_t);
uint8_t swu_rx_rd_chr(void);
void swu_rx_isr(void);
void swu_init(void);
void uart_init(void);

void BOARD_InitPins(void);
uint32_t LPUART0_GetFreq(void);

#endif


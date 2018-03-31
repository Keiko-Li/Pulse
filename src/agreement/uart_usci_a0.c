#include <msp430.h>
/*
 * ---------UART---------
 * PORT		TYPE	PIN
 * TXD		OUT		P3.3
 * RXD		IN		P3.4
 * ---------------------
 */
#define UART_SET_PIN()   {\
                            P3SEL |= BIT3 + BIT4;\
                         }

static unsigned char *uart_tx_buff;
static unsigned char *uart_rx_buff;

static unsigned char uart_tx_num = 0;
static unsigned char uart_rx_num = 0;

static void (*uart_rx_func)(void);

void uart_init(void)
{
    UART_SET_PIN();

    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0 = 217;                            // 25MHz 115200bps
    UCA0BR1 = 0;                              // 25MHz 115200bps
    UCA0MCTL |= UCBRS_0 + UCBRF_0;            // 25MHz 115200bps

    UCA0CTL1 &= ~UCSWRST;
    UCA0IFG  &=~(UCTXIFG + UCRXIFG);
}

unsigned char uart_transmit_frame(unsigned char *p_buff, unsigned char num)
{
	if (UCA0STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCA0IE |= UCTXIE;
	__enable_interrupt();
    uart_tx_buff = p_buff;
    uart_tx_num  = num;
	UCA0TXBUF = *uart_tx_buff++;
    __bis_SR_register(LPM0_bits);
	return 1;
}

unsigned char uart_receive_frame(void (*p_func)(void), unsigned char *p_buff, unsigned char num)
{
    __disable_interrupt();
    UCA0IE |= UCRXIE;
    __enable_interrupt();
    uart_rx_func = p_func;
    uart_rx_buff = p_buff;
    uart_rx_num  = num;
    return 1;
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4)) {
        case  0: break;                           // Vector  0: No interrupts
        case  2:                                  // Vector  2: RXIFG
            *uart_rx_buff++ = UCA0RXBUF;
            uart_rx_num--;
            if (!uart_rx_num) {
                UCA0IE &=~UCRXIE;
                (*uart_rx_func)();
            }
            break;
        case  4:                                  // Vector  4: TXIFG
            uart_tx_num--;
            if (uart_tx_num) {
                UCA0TXBUF = *uart_tx_buff++;
            }
            else {
                UCA0IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
    }
}

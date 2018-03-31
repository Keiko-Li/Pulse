#include <msp430.h>
#include "inc/agreement/spi.h"
/*
 * ---------SPI---------
 * PORT		TYPE	PIN
 * SIMO		OUT		P3.3
 * SOMI		IN		P3.4
 * SCK      OUT     P2.7
 * ---------------------
 */
#define SPI_SET_PIN()   {\
                            P3SEL |= BIT3 + BIT4;\
                            P2SEL |= BIT7;\
                         }

static unsigned char *spi_tx_buff;
static unsigned char *spi_rx_buff;

static unsigned char spi_tx_num = 0;
static unsigned char spi_rx_num = 0;

void spi_init(void)
{
    SPI_SET_PIN();

    UCA0CTL1 |= UCSWRST;
    UCA0CTL0 |= UCMST + UCMODE_0 + UCSYNC + UCCKPL + UCMSB;
    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0  = 1;
    UCA0BR1  = 0;
    UCA0MCTL = 0;

    UCA0CTL1 &=~UCSWRST;
    UCA0IFG  &=~(UCTXIFG + UCRXIFG);
}

unsigned char spi_transmit_frame(unsigned char *p_buff, unsigned char num)
{
	if (UCA0STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCA0IE |= UCTXIE;
	UCA0IE &=~UCRXIE;
	__enable_interrupt();
    spi_tx_buff = p_buff;
    spi_tx_num  = num;
	UCA0TXBUF = *spi_tx_buff++;
    __bis_SR_register(LPM0_bits);
	return 1;
}

unsigned char spi_receive_frame(unsigned char *p_buff, unsigned char num)
{
    if (UCA0STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCA0IE &=~UCTXIE;
    UCA0IE |= UCRXIE;
    __enable_interrupt();
    spi_rx_buff = p_buff;
    spi_rx_num  = num;
    UCA0TXBUF = 0xff;
    __bis_SR_register(LPM0_bits);
	return 1;
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4)) {
        case  0: break;                           // Vector  0: No interrupts
        case  2:                                  // Vector  2: RXIFG
            *spi_rx_buff++ = UCA0RXBUF;
            spi_rx_num--;
            if (spi_rx_num) {
                UCA0TXBUF = 0xff;
            }
            else {
                UCA0IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case  4:                                  // Vector  4: TXIFG
            UCA0RXBUF;
            spi_tx_num--;
            if (spi_tx_num) {
                UCA0TXBUF = *spi_tx_buff++;
            }
            else {
                UCA0IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
    }
}

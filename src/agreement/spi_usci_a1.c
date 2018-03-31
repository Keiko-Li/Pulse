#include <msp430.h>
/*
 * ---------SPI---------
 * PORT		TYPE	PIN
 * SIMO		OUT		P4.4
 * SOMI		IN		P4.5
 * SCK      OUT     P4.0
 * ---------------------
 */
#define SPI_SET_PIN()   {\
                            P4SEL |= BIT0 + BIT4 + BIT5;\
                         }

static unsigned char *spi_tx_buff;
static unsigned char *spi_rx_buff;

static unsigned char spi_tx_num = 0;
static unsigned char spi_rx_num = 0;

void spi_init(void)
{
    SPI_SET_PIN();

    UCA1CTL1 |= UCSWRST;
    UCA1CTL0 |= UCMST + UCMODE_0 + UCSYNC + UCCKPL + UCMSB;
    UCA1CTL1 |= UCSSEL_2;

    UCA1BR0  = 1;
    UCA1BR1  = 0;
    UCA1MCTL = 0;

    UCA1CTL1 &=~UCSWRST;
    UCA1IFG  &=~(UCTXIFG + UCRXIFG);
}

unsigned char spi_transmit_frame(unsigned char *p_buff, unsigned char num)
{
	if (UCA1STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCA1IE |= UCTXIE;
	UCA1IE &=~UCRXIE;
	__enable_interrupt();
    spi_tx_buff = p_buff;
    spi_tx_num  = num;
	UCA1TXBUF = *spi_tx_buff++;
    __bis_SR_register(LPM0_bits);
	return 1;
}

unsigned char spi_receive_frame(unsigned char *p_buff, unsigned char num)
{
    if (UCA1STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCA1IE &=~UCTXIE;
    UCA1IE |= UCRXIE;
    __enable_interrupt();
    spi_rx_buff = p_buff;
    spi_rx_num  = num;
    UCA1TXBUF = 0xff;
    __bis_SR_register(LPM0_bits);
	return 1;
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch (__even_in_range(UCA1IV, 4)) {
        case  0: break;                           // Vector  0: No interrupts
        case  2:                                  // Vector  2: RXIFG
            *spi_rx_buff++ = UCA1RXBUF;
            spi_rx_num--;
            if (spi_rx_num) {
                UCA1TXBUF = 0xff;
            }
            else {
                UCA1IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case  4:                                  // Vector  4: TXIFG
            UCA1RXBUF;
            spi_tx_num--;
            if (spi_tx_num) {
                UCA1TXBUF = *spi_tx_buff++;
            }
            else {
                UCA1IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
    }
}

#include <msp430.h>
/*
 * ---------SPI---------
 * PORT		TYPE	PIN
 * SIMO		OUT		P3.0
 * SOMI		IN		P3.1
 * SCK      OUT     P3.2
 * ---------------------
 */
#define SPI_SET_PIN()   {\
                            P3SEL |= BIT0 + BIT1 + BIT2;\
                         }

static unsigned char *spi_tx_buff;
static unsigned char *spi_rx_buff;

static unsigned char spi_tx_num = 0;
static unsigned char spi_rx_num = 0;

void spi_init(void)
{
    SPI_SET_PIN();

    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 |= UCMST + UCMODE_0 + UCSYNC + UCCKPL + UCMSB;
    UCB0CTL1 |= UCSSEL_2;

    UCB0BR0  = 1;
    UCB0BR1  = 0;

    UCB0CTL1 &=~UCSWRST;
    UCB0IFG  &=~(UCTXIFG + UCRXIFG);
}

unsigned char spi_transmit_frame(unsigned char *p_buff, unsigned char num)
{
	if (UCB0STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCB0IE |= UCTXIE;
	UCB0IE &=~UCRXIE;
	__enable_interrupt();
    spi_tx_buff = p_buff;
    spi_tx_num  = num;
	UCB0TXBUF = *spi_tx_buff++;
    __bis_SR_register(LPM0_bits);
	return 1;
}

unsigned char spi_receive_frame(unsigned char *p_buff, unsigned char num)
{
    if (UCB0STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCB0IE &=~UCTXIE;
    UCB0IE |= UCRXIE;
    __enable_interrupt();
    spi_rx_buff = p_buff;
    spi_rx_num  = num;
    UCB0TXBUF = 0xff;
    __bis_SR_register(LPM0_bits);
	return 1;
}

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
    switch (__even_in_range(UCB0IV, 4)) {
        case  0: break;                           // Vector  0: No interrupts
        case  2:                                  // Vector  2: RXIFG
            *spi_rx_buff++ = UCB0RXBUF;
            spi_rx_num--;
            if (spi_rx_num) {
                UCB0TXBUF = 0xff;
            }
            else {
                UCB0IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case  4:                                  // Vector  4: TXIFG
            UCB0RXBUF;
            spi_tx_num--;
            if (spi_tx_num) {
                UCB0TXBUF = *spi_tx_buff++;
            }
            else {
                UCB0IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
    }
}

#include <msp430.h>
/*
 * ---------SPI---------
 * PORT		TYPE	PIN
 * SIMO		OUT		P4.1
 * SOMI		IN		P4.2
 * SCK      OUT     P4.3
 * ---------------------
 */
#define SPI_SET_PIN()   {\
                            P4SEL |= BIT1 + BIT2 + BIT3;\
                         }

static unsigned char *spi_tx_buff;
static unsigned char *spi_rx_buff;

static unsigned char spi_tx_num = 0;
static unsigned char spi_rx_num = 0;

void spi_init(void)
{
    SPI_SET_PIN();

    UCB1CTL1 |= UCSWRST;
    UCB1CTL0 |= UCMST + UCMODE_0 + UCSYNC + UCCKPL + UCMSB;
    UCB1CTL1 |= UCSSEL_2;

    UCB1BR0  = 1;
    UCB1BR1  = 0;

    UCB1CTL1 &=~UCSWRST;
    UCB1IFG  &=~(UCTXIFG + UCRXIFG);
}

unsigned char spi_transmit_frame(unsigned char *p_buff, unsigned char num)
{
	if (UCB1STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCB1IE |= UCTXIE;
	UCB1IE &=~UCRXIE;
	__enable_interrupt();
    spi_tx_buff = p_buff;
    spi_tx_num  = num;
	UCB1TXBUF = *spi_tx_buff++;
    __bis_SR_register(LPM0_bits);
	return 1;
}

unsigned char spi_receive_frame(unsigned char *p_buff, unsigned char num)
{
    if (UCB1STAT & UCBUSY) return 0;
	__disable_interrupt();
    UCB1IE &=~UCTXIE;
    UCB1IE |= UCRXIE;
    __enable_interrupt();
    spi_rx_buff = p_buff;
    spi_rx_num  = num;
    UCB1TXBUF = 0xff;
    __bis_SR_register(LPM0_bits);
	return 1;
}

#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
    switch (__even_in_range(UCB1IV, 4)) {
        case  0: break;                           // Vector  0: No interrupts
        case  2:                                  // Vector  2: RXIFG
            *spi_rx_buff++ = UCB1RXBUF;
            spi_rx_num--;
            if (spi_rx_num) {
                UCB1TXBUF = 0xff;
            }
            else {
                UCB1IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case  4:                                  // Vector  4: TXIFG
            UCB1RXBUF;
            spi_tx_num--;
            if (spi_tx_num) {
                UCB1TXBUF = *spi_tx_buff++;
            }
            else {
                UCB1IFG &=~UCTXIFG;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
    }
}

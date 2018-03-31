#include <msp430.h>
/*
 * ---------I2C---------
 * PORT		TYPE	PIN
 * SDA      INOUT   P4.1
 * SCL		OUT		P4.2
 * ---------------------
 */
#define I2C_SET_PIN()   {\
                              P4SEL |= BIT1 + BIT2;\
                         }

static unsigned char *i2c_tx_buff;
static unsigned char *i2c_rx_buff;

static unsigned char i2c_tx_num = 0;
static unsigned char i2c_rx_num = 0;

void i2c_init(void)
{
    I2C_SET_PIN();

    UCB1CTL1 |= UCSWRST;
    UCB1CTL0 |= UCMST + UCMODE_3 + UCSYNC;
    UCB1CTL1 |= UCSSEL_2;

    UCB1BR0 = 16;
    UCB1BR1 = 0;

    UCB1CTL1 &=~UCSWRST;
    UCB1IFG  &=~(UCTXIFG + UCRXIFG);
}

unsigned char i2c_transmit_frame(unsigned char slave_addr, unsigned char *p_buff,unsigned char num)
{
    if (UCB1STAT & UCBUSY) return 0;
    __disable_interrupt();
    UCB1IE |= UCTXIE;
    UCB1IE &=~UCRXIE;
    __enable_interrupt();
    i2c_tx_buff = p_buff;
    i2c_tx_num  = num;
    UCB1I2CSA = slave_addr;
    UCB1CTL1 |= UCTR + UCTXSTT;
    __bis_SR_register(LPM0_bits);
    return 1;
}

unsigned char i2c_receive_frame(unsigned char slave_addr, unsigned char *p_buff, unsigned char num)
{
    if (UCB1STAT & UCBUSY) return 0;
    __disable_interrupt();
    UCB1IE &=~UCTXIE;
    UCB1IE |= UCRXIE;
    __enable_interrupt();
    i2c_rx_buff = p_buff;
    i2c_rx_num  = num;
    UCB1I2CSA = slave_addr;
    UCB1CTL1 &=~UCTR;
    UCB1CTL1 |= UCTXSTT;
    __bis_SR_register(LPM0_bits);
    return 1;
 }

#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
    switch (__even_in_range(UCB1IV, 12)) {
        case  0: break;                           // Vector  0: No interrupts
        case  2: break;                           // Vector  2: ALIFG
        case  4: break;                           // Vector  4: NACKIFG
        case  6: break;                           // Vector  6: STTIFG
        case  8: break;                           // Vector  8: STPIFG
        case 10:                                  // Vector 10: RXIFG
            *i2c_rx_buff++ = UCB1RXBUF;
            i2c_rx_num--;
            if (i2c_rx_num) {
              if (i2c_rx_num == 1) {
                  UCB1CTL1 |= UCTXSTP;
              }
            }
            else {
              __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case 12:                                  // Vector 12: TXIFG
            if (i2c_tx_num) {
              UCB1TXBUF = *i2c_tx_buff++;
              i2c_tx_num--;
            }
            else {
              UCB1CTL1 |= UCTXSTP;
              UCB1IFG  &=~UCTXIFG;
              __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        default: break;
    }
}

#include <msp430.h>
/*
 * ---------SPI---------
 * PORT     TYPE    PIN
 * SIMO     OUT     P3.3
 * SOMI     IN      P3.4
 * SCK      OUT     P2.7
 * ---------------------
 */
#define SPI_SET_PIN()   {\
                            P3DIR |= BIT3;\
                            P3DIR &=~BIT4;\
                            P2DIR |= BIT7;\
                         }
#define SPI_SIMO_SET()  P3OUT |= BIT3
#define SPI_SIMO_CLR()  P3OUT &=~BIT3
#define SPI_SOMI_IN()   P3IN  &  BIT4
#define SPI_CLK_SET()   P2OUT |= BIT7
#define SPI_CLK_CLR()   P2OUT &=~BIT7

#define F_CPU  25000000

static inline void delay_us(void)
{
    __delay_cycles(F_CPU/25000000);
}

void spi_init(void)
{
    SPI_SET_PIN();
}

void spi_transmit_char(unsigned char data)
{
	unsigned char i=0;
	for (i=8; i>0; i--) {
        delay_us();
        SPI_CLK_CLR();
        if (data & BIT7) {
            SPI_SIMO_SET();
        }
        else {
            SPI_SIMO_CLR();
        }
        data <<= 1;
        delay_us();
		SPI_CLK_SET();
	}
}

unsigned char spi_receive_char(void)
{
	unsigned char i=0;
	unsigned char temp=0;
	for (i=8; i>0; i--) {
	    delay_us();
		SPI_CLK_SET();
		temp <<= 1;
		if (SPI_SOMI_IN()) temp |= BIT0;
        delay_us();
        SPI_CLK_CLR();
	}
	return temp;
}

unsigned char spi_transmit_frame(unsigned char *p_buff, unsigned int num)
{
	unsigned char i=0;
	__disable_interrupt();
	for (i=num; i>0; i--) {
		spi_transmit_char(*p_buff);
		p_buff++;
	}
	__enable_interrupt();
	return 1;
}

unsigned char spi_receive_frame(unsigned char *p_buff, unsigned int num)
{
	unsigned char i=0;
	__disable_interrupt();
	for (i=num; i>0; i--) {
		 *p_buff = spi_receive_char();
		 p_buff++;
	}
	__enable_interrupt();
	return 1;
}

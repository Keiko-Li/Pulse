/*
 * max30102.c
 *
 *  Created on: 2017��3��10��
 *      Author: Administrator
 */
#include <msp430.h>
#include "inc/agreement/i2c.h"
#define max30102_WR_addr	0x57
#define INTERRUPT_REG 		0X00
void max30102_init(void)
{
	i2c_transmit_frame(max30102_WR_addr,"\x09\x0a",2);
	i2c_transmit_frame(max30102_WR_addr,"\x01\xF0",2);
	i2c_transmit_frame(max30102_WR_addr,"\x00\x00",2);
	i2c_transmit_frame(max30102_WR_addr,"\x03\x02",2);
	i2c_transmit_frame(max30102_WR_addr,"\x21\x01",2);

	i2c_transmit_frame(max30102_WR_addr,"\x04\x00",2);
	i2c_transmit_frame(max30102_WR_addr,"\x05\x00",2);
	i2c_transmit_frame(max30102_WR_addr,"\x06\x00",2);

	i2c_transmit_frame(max30102_WR_addr,"\x0a\x47",2);
	i2c_transmit_frame(max30102_WR_addr,"\x0c\x47",2);
	i2c_transmit_frame(max30102_WR_addr,"\x0d\x47",2);
}
//------
void max30102_Bus_Read(unsigned char RegAddr,unsigned char *data)
{
	i2c_start();
	i2c_transmit_char(max30102_WR_addr<<1);
	i2c_transmit_char(RegAddr);
	i2c_receive_frame(max30102_WR_addr,data,1);
}
//-------
void max30102_FIFO_Read(unsigned char RegAddr,unsigned char *data)
{
	i2c_start();
	i2c_transmit_char(max30102_WR_addr<<1);
	i2c_transmit_char(RegAddr);
	i2c_receive_frame(max30102_WR_addr,data,18);
}


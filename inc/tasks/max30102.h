/*
 * max30102.h
 *
 *  Created on: 2017Äê3ÔÂ10ÈÕ
 *      Author: Administrator
 */

#ifndef SRC_LCD_MAX30102_H_
#define SRC_LCD_MAX30102_H_

void max30102_init(void);
void max30102_Bus_Read(unsigned char RegAddr,unsigned char *data);
void max30102_FIFO_Read(unsigned char RegAddr,unsigned char *data);

#endif /* SRC_LCD_MAX30102_H_ */

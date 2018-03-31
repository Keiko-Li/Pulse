/*
 * lcd12864.h
 *
 *  Created on: 2015-6-25
 *      Author: redchenjs
 */

#ifndef LCD12864_H_
#define LCD12864_H_

#define SPI_MODE
//#define M68_MODE

void LCD_Del_Word(char num);

void LCD_CLR_Char(unsigned char x, unsigned char y);
void LCD_CLR_Word(unsigned char x, unsigned char y);
void LCD_CLR_DDRAM(void);

void LCD_CLR_Line(unsigned char x, unsigned char y, unsigned char length, unsigned char dir);
void LCD_CLR_GDRAM(void);

void LCD_Disp_Char(unsigned char data, unsigned char x, unsigned char y);
void LCD_Disp_Word(unsigned char data[2], unsigned char x, unsigned char y);
void LCD_Disp_String(char *str, unsigned char x, unsigned char y);

void LCD_Disp_Image(const unsigned char *str, unsigned char x, unsigned char y, unsigned char width, unsigned char height);
void LCD_Disp_Point(unsigned char color, unsigned char x,unsigned char y);
void LCD_Fill_GDRAM(void);

void LCD_Disp_CGRAM(unsigned char index, unsigned char x,unsigned char y);

void LCD_Disp_Init(void);

void LCD_Draw_Line_x(unsigned char X0, unsigned char X1, unsigned char Y, unsigned char Color);
void LCD_Draw_Line_Y(unsigned char X, unsigned char Y0, unsigned char Y1, unsigned char Color);
void LCD_Draw_Line(unsigned char StartX, unsigned char StartY, unsigned char EndX, unsigned char EndY, unsigned char Color);
void fsin2(void);

extern const unsigned char qrcode[];
extern const unsigned char logo[];

extern char tab_SPI1[17];
extern char tab_SPI2[17];
extern char tab_SPI3[17];
extern char tab_SPI4[17];

#endif /* LCD12864_H_ */


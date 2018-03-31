/*
 * lcd12864.c
 *
 *  Created on: 2016-6-25
 *      Author: redchenjs
 *
 *      功能说明：LCD12864硬件驱动函数，包括LCD初始化，显示文字，图片等功能。
 *      引脚配置：
 *      	M68并口：RS->P6.6, RW->P6.5, EN->P6.4, DATA->P3（本c文件中定义）
 *      	SPI串口：CS->P6.6, SID->P6.5, CLK->P6.4(在spi.h中定义)
 *      注：切换通信协议直接修改lcd12864.h中的条件编译指令，函数完全兼容
 */
#include "msp430.h"
#include "lcd12864.h"

const unsigned char CGRAM[4][32]={	0x0F,0xF0,0x10,0x08,0x20,0x04,0x40,0x02,0x9C,0x39,0xBE,0x7D,0x80,0x01,0x80,0x01,
									0x80,0x01,0x88,0x11,0x84,0x21,0x43,0xC2,0x20,0x04,0x10,0x08,0x0F,0xF0,0x00,0x00,

									0x00,0x00,0x10,0x02,0x18,0x04,0x06,0x08,0x00,0x00,0x00,0x00,0xFF,0xCE,0x00,0x00,
									0x00,0x00,0x00,0x20,0x00,0x50,0x00,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

									0x01,0x80,0x01,0x80,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0x01,0x80,0x01,0x80,
									0x01,0x80,0x01,0x80,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0x01,0x80,0x01,0x80,

									0xFF,0xFF,0xFF,0xFF,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xFF,0xFF,
									0xFF,0xFF,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xC1,0x83,0xFF,0xFF,0xFF,0xFF};

#define CPU_FREQ ((double)16000000)
#define delay_ns(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000000000.0))
#define delay_us(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000.0))

char tab_SPI1[17]={"0123456789ABCDEF"};
char tab_SPI2[17]={"abcdef0123456789"};
char tab_SPI3[17]={"一个三四五六七八"};
char tab_SPI4[17]={"诶必西地易艾芙季"};

/*12864基础应用指令*/
#define CLEAR_SCREEN        0x01                    //清屏指令：清屏且AC值为00H
#define AC_INIT             0x02                    //将AC设置为00H。且游标移到原点位置
#define CURSE_ADD        	0x06                    //设定游标移到方向及图像整体移动方向（默认游标右移，图像整体不动）
#define FUN_MODE        	0x30                    //工作模式：8位基本指令集
#define DISPLAY_ON        	0x0c                    //显示开,显示游标，且游标位置反白
#define DISPLAY_OFF        	0x08                    //显示关
#define CURSE_DIR        	0x14                    //游标向右移动:AC=AC+1
#define SET_CG_AC        	0x40                    //设置AC，范围为：00H~3FH
#define SET_DD_AC        	0x80
#define BUSY_FLAG			0x80
/*12864扩展应用指令*/
#define SP_MODE      		0x34					// 8位数据，基本指令操作
#define GRAPON    			0x36					// 扩充指令，图形显示开
#define	gdbas     			0x80
//*******************************************************************
#define CGRAM_0				0x00
#define CGRAM_1				0x02
#define CGRAM_2				0x04
#define CGRAM_3				0x06

#ifdef M68_MODE										//M68并口通信模式

#define SET_CMD_PORT  	P6DIR |= 0x70; P8DIR |= BIT2; P8OUT |= BIT2
													//设置命令口方向
#define SET_DATA_IN		P3DIR  = 0x00				//切换数据口方向，输入模式
#define SET_DATA_OUT	P3DIR  = 0xff				//切换数据口方向，输出模式
#define LCD_DATA_IN		P3IN						//选择LCD数据口输入寄存器
#define LCD_DATA_OUT	P3OUT						//选择LCD数据口输出寄存器
#define LCD_RS_H		P6OUT |= BIT6				//RS寄存器选择，选中数据寄存器
#define LCD_RS_L		P6OUT &=~BIT6				//RS寄存器选择，选中指令寄存器
#define LCD_RW_H		P6OUT |= BIT5				//RW读写位设置，选中读模式
#define LCD_RW_L		P6OUT &=~BIT5				//RW读写位设置，选中写模式
#define LCD_EN_H		P6OUT |= BIT4				//EN使能信号，使能有效
#define LCD_EN_L		P6OUT &=~BIT4				//EN使能信号，使能无效

/*******************************************
函数名称：WaitForReady
函数功能：并口模式判忙
输入参数：无
输出参数：无
函数返回：无
********************************************/
void WaitForReady(void)
{
	unsigned char ReadTemp = 0;

	LCD_RS_L;										//选中指令寄存器
	LCD_RW_H;										//选中读取模式
    SET_DATA_IN;									//切换数据口到输入模式

    do {
        LCD_EN_H;									//使能打开
       	_NOP();										//等待更新
       	ReadTemp = LCD_DATA_IN;						//读取数据口
       	LCD_EN_L;									//使能关闭
    }while(ReadTemp & BUSY_FLAG);					//判忙等待
}
/*******************************************
函数名称：SendByte
函数功能：向LCD发送一个字节的数据
输入参数：data-->字节数据
输出参数：无
函数返回：无
********************************************/
void SendByte(unsigned char data)
{
	WaitForReady();									//判忙等待

	SET_DATA_OUT;									//切换数据口到输出模式

    LCD_RW_L;										//选中写入模式
    LCD_DATA_OUT = data;							//更新数据口

    LCD_EN_H;										//使能打开
    _NOP();											//等待更新
    LCD_EN_L;										//使能关闭
}
/*******************************************
函数名称：SendCMD
函数功能：向LCD发送一帧命令
输入参数：data-->字节数据
输出参数：无
函数返回：无
********************************************/
void SendCMD(unsigned char data)
{
	WaitForReady();									//判忙等待

	SET_DATA_OUT;									//切换数据口到输出模式

	LCD_RS_L;										//选中指令寄存器
    LCD_RW_L;										//选中写入模式
    LCD_DATA_OUT = data;							//更新数据口

    LCD_EN_H;										//使能打开
    _NOP();											//等待更新
    LCD_EN_L;										//使能关闭
}
/*******************************************
函数名称：SendData
函数功能：向LCD发送一帧数据
输入参数：data-->字节数据
输出参数：无
函数返回：无
********************************************/
void SendData(unsigned char data)
{
	WaitForReady();									//判忙等待

	SET_DATA_OUT;									//切换数据口到输出模式

	LCD_RS_H;										//选中数据寄存器
    LCD_RW_L;										//选中写入模式
    LCD_DATA_OUT = data;							//更新数据口

    LCD_EN_H;										//使能打开
    _NOP();											//等待更新
    LCD_EN_L;										//使能关闭
}
/*******************************************
//        读数据
********************************************/
unsigned char ReceiveByte(void)
{
    unsigned char ReadData = 0;

	WaitForReady();									//判忙等待

    SET_DATA_IN;									//切换数据口到输入模式

    LCD_RS_H;										//选中数据寄存器
    LCD_RW_H;										//选中读模式

    LCD_EN_H;										//使能打开
   	_NOP();											//等待更新
   	ReadData = LCD_DATA_IN;							//读取数据口
    LCD_EN_L;										//使能关闭

    return ReadData;
}
#endif

#ifdef SPI_MODE

#include "spi.h"

#define SET_CMD_PORT  	P6DIR |= 0x70; P8DIR |= BIT2; P8OUT &=~BIT2		//设置命令口方向

unsigned char GDRAM_buff[1024] = {0};									//全屏GDRAM缓冲
unsigned char DDRAM_buff[2048] = {0};									//全屏DDRAM缓冲

void SendCMD(unsigned char dat)	//写控制命令
{
	unsigned char conf[3]={0};

	conf[0] = 0xf8;//11111,00,0 RW=0,RS=0 同步标志
	conf[1] = dat&0xf0;//高四位
	conf[2] = (dat&0x0f)<<4;//低四位

	SPI_CS_High();
	SPI_TxFrame(conf, 3);
	SPI_CS_Low();
}

void SendData(unsigned char dat)	//写显示数据或单字节字符
{

	unsigned char conf[3]={0};

	conf[0] = 0xfa;//11111,01,0 RW=0,RS=1
	conf[1] = dat&0xf0;//高四位
	conf[2] = (dat&0x0f)<<4;//低四位

	SPI_CS_High();
	SPI_TxFrame(conf, 3);
	SPI_CS_Low();
}

unsigned char ReceiveByte(void)
{
	unsigned char temp;

	SPI_CS_High();
	SPI_RxFrame(&temp, 1);
	SPI_CS_Low();

	return temp;
}

#endif
/*
 * 设置当前显存游标地址
 *
 * x：1-8(每个地址两个字节)
 * y：1-4(4行)
 */
void SetCoord(unsigned char x, unsigned char y)
{
   switch (y) {
		case 1:	SendCMD(0x7F + x);	break;
		case 2: SendCMD(0x8F + x);	break;
		case 3: SendCMD(0x87 + x);	break;
		case 4: SendCMD(0x97 + x);	break;
		default:					break;
   }
}
/*
 * x:1-16
 * y:1-4
 */
void ReadChar(unsigned char *data, unsigned char x, unsigned char y)
{
	unsigned char DDRAM_hbit, DDRAM_lbit;

	SendCMD(0x30);
	SetCoord((x-1)/2+1, y);
	ReceiveByte();				//预读数据
	DDRAM_hbit=ReceiveByte();	//读取当前显示高8 位数据
	DDRAM_lbit=ReceiveByte();	//读取当前显示低8 位数据

	if (x%2)
		*data = DDRAM_lbit;
	else
		*data = DDRAM_hbit;
}

/*
 * x:1-8
 * y:1-4
 */
void ReadWord(unsigned char *data, unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
	ReceiveByte();				//预读数据
	data[1] = ReceiveByte();	//读取当前显示高8 位数据
	data[0] = ReceiveByte();	//读取当前显示低8 位数据
}

void WriteGDRAM(unsigned char data)
{
	unsigned char i,j,k;
	unsigned char bGDRAMAddrX = 0x80; //GDRAM 水平地址
	unsigned char bGDRAMAddrY = 0x80; //GDRAM 垂直地址
	for (i=0;i<2;i++) {
		for (j=0;j<32;j++) {
			for (k=0;k<8;k++) {
				SendCMD(0x34); 			//设置为8 位MPU 接口，扩充指令集,绘图模式关
				SendCMD(bGDRAMAddrY+j); //垂直地址Y
				SendCMD(bGDRAMAddrX+k); //水平地址X
				SendData(data);
				SendData(data);
			}
		}
		bGDRAMAddrX = 0x88;
	}
	SendCMD(0x36); //打开绘图模式
	SendCMD(0x30); //恢复基本指令集，关闭绘图模式
}

/*
 * 向LCD控制器写入自定义字符
 */
void WriteCGRAM(void)
{
     int i, j;

     for (j=0; j<4; j++) {
    	 SendCMD(0x30);
    	 SendCMD(0x40+j*16);
		 for (i=0;i<16;i++) {
			 SendData(CGRAM[j][i*2]);
			 SendData(CGRAM[j][i*2+1]);
		 }
     }
}

/*
 * 显示用户自定义字符
 * index：1-4
 * x：1-8(每个字符2个字节宽度)
 * y：1-4
 */
void LCD_Disp_CGRAM(const unsigned char index, unsigned char x,unsigned char y)
{
	SetCoord(x, y);
	SendData(0x00);
	SendData(index*2);
}

void LCD_Disp_Image(const unsigned char *str, unsigned char x, unsigned char y, unsigned char width, unsigned char height)
{
	unsigned char i=0, j=0;
	unsigned char x_start=0, x_width=0;
	unsigned char y_upper_start=0, y_upper_end=0;
	unsigned char y_lower_start=0, y_lower_end=0;

	x_start = x / 8;
	x_width = width % 8 ? width / 8 + 1 : width / 8;

	y_upper_start = y < 32 ? y : 32;
	y_upper_end	= y + height < 32 ? y + height : 32;
	y_lower_start = y < 32 ? 0 : y - 32;
	y_lower_end = y + height < 32 ? 0 : y + height - 32;

	SendCMD(0x36);	//绘图显示开，扩充指令集extended instruction(DL=8BITS,RE=1,G=1)

	for (i=y_upper_start; i<y_upper_end; i++) {
		SendCMD(0x80 + i);				//SET  垂直地址 VERTICAL ADD
		SendCMD(0x80 + x_start);		//SET  水平地址 HORIZONTAL ADD
		for (j=0; j<x_width; j++) {
			SendData(*str++);
		}
	}

	for (i=y_lower_start; i<y_lower_end; i++) {
		SendCMD(0x80 + i);				//SET 垂直地址 VERTICAL ADD
		SendCMD(0x88 + x_start);		//SET 水平地址 HORIZONTAL ADD
		for (j=0; j<x_width; j++) {
			SendData(*str++);
		}
	}
}

void LCD_CLR_Line(unsigned char x, unsigned char y, unsigned char length, unsigned char dir)
{
	const unsigned char clr[128]={0};
	if (dir)
		LCD_Disp_Image(clr, x, y, length, 1);
	else
		LCD_Disp_Image(clr, x, y, 1, length);
}
/*
 * x:1-16
 * y:1-4
 */
void LCD_Disp_Char(unsigned char data, unsigned char x, unsigned char y)
{
	unsigned char temp;

	SendCMD(0x30);
	ReadChar(&temp, x, y);
	SetCoord((x-1)/2+1, y);
	if (x%2) {
		SendData(data);
		SendData(temp);
	}
	else {
		SendData(temp);
		SendData(data);
	}
}

void LCD_CLR_Char(unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
    SendData(0x20);
}

void LCD_Disp_Word(unsigned char *data, unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
    SendData(data[0]);
    SendData(data[1]);
}

void LCD_CLR_Word(unsigned char x, unsigned char y)
{
	SendCMD(0x30);
	SetCoord(x, y);
    SendData(0x20);
    SendData(0x20);
}

void LCD_Del_Word(char num)
{
	unsigned char ReadTemp=0;

	SendCMD(0x30);
	SendCMD(0x07);   //游标右移

	ReceiveByte();
	ReadTemp = ReceiveByte();

	SendCMD(ReadTemp);

	while (num--) {
		SendData(0x20);
		SendData(0x20);
	}
}

void LCD_Disp_String(char *str, unsigned char x, unsigned char y)
{
	unsigned char temp;
	SendCMD(0x30);
	SetCoord(x, y);
	temp = *str;
	while (temp != 0) {
		SendData(temp);
		temp = *++str;
	}
}

void LCD_Disp_Point(unsigned char color, unsigned char x,unsigned char y)
{
	unsigned char x_Dyte,x_byte; //定义列地址的字节位，及在字节中的哪1 位
	unsigned char y_Dyte,y_byte; //定义为上下两个屏(取值为0，1)，行地址(取值为0~31)
	unsigned char GDRAM_hbit,GDRAM_lbit;

	SendCMD(0x36); //扩展指令命令
	/***X,Y 坐标互换，即普通的X,Y 坐标***/
	x_Dyte=x/16; //计算在16 个字节中的哪一个
	x_byte=x&0x0f; //计算在该字节中的哪一位
	y_Dyte=y/32; //0 为上半屏，1 为下半屏
	y_byte=y&0x1f; //计算在0~31 当中的哪一行
	SendCMD(0x80+y_byte); //设定行地址(y 坐标),即是垂直地址
	SendCMD(0x80+x_Dyte+8*y_Dyte); //设定列地址(x 坐标)，并通过8*y_Dyte 选定上下屏，即是水平地址
	ReceiveByte(); //预读取数据
	GDRAM_hbit=ReceiveByte(); //读取当前显示高8 位数据
	GDRAM_lbit=ReceiveByte(); //读取当前显示低8 位数据
	delay_ms(1);
	SendCMD(0x80+y_byte); //设定行地址(y 坐标)
	SendCMD(0x80+x_Dyte+8*y_Dyte); //设定列地址(x 坐标)，并通过8*y_Dyte 选定上下屏
	delay_ms(1);
	if(x_byte<8) //判断其在高8 位，还是在低8 位
	{
		if(color==1)
		{
			SendData(GDRAM_hbit|(0x01<<(7-x_byte))); //置位GDRAM 区高8 位数据中相应的点
		}
		else
			SendData(GDRAM_hbit&(~(0x01<<(7-x_byte)))); //清除GDRAM 区高8 位数据中相应的点

			SendData(GDRAM_lbit); //显示GDRAM 区低8 位数据
		}
		else
		{
			SendData(GDRAM_hbit);         //写高8位数据
		if(color==1)
			SendData(GDRAM_lbit|(0x01<<(15-x_byte))); //置位GDRAM 区高8 位数据中相应的点
		else
			SendData(GDRAM_lbit&(~(0x01<<(15-x_byte))));//清除GDRAM区高8位数据中相应的点
	}
	SendCMD(0x30); //恢复到基本指令集
}

void LCD_CLR_GDRAM(void)
{
	WriteGDRAM(0x00);
}

void LCD_Fill_GDRAM(void)
{
	WriteGDRAM(0xff);
}

void LCD_CLR_DDRAM(void)
{
	SendCMD(0x01);
	SendCMD(0x34);
	SendCMD(0x30);
}

void LCD_Disp_Init(void)
{
	SET_CMD_PORT;  //初始化命令口

    delay_ms(500);
    SendCMD(0x30);   //基本指令集
    delay_ms(1);
    SendCMD(0x02);   // 地址归位
    delay_ms(1);
	SendCMD(0x0c);   //整体显示打开,游标关闭
    delay_ms(1);
	SendCMD(0x01);   //清除显示
    delay_ms(10);
	SendCMD(0x06);   //游标右移
    delay_ms(1);
	SendCMD(0x80);   //设定显示的起始地址

	WriteCGRAM();

	LCD_CLR_GDRAM();
	LCD_CLR_DDRAM();
}

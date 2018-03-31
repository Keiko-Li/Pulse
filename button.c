/*
 * button.c
 *
 *  Created on: 2018年3月15日
 *      Author: laceyli
 */
#include <msp430.h>
#include "inc/user/System.h"

extern unsigned char flag;

void GPIO_init()
{
    P1REN |= BIT1;                      //启用P1.1内部上下拉电阻
    P1OUT |= BIT1;                      //将电阻设置为上拉
    P2REN |= BIT1;                      //启用P1.1内部上下拉电阻
    P2OUT |= BIT1;                      //将电阻设置为上拉

    P1DIR &= ~BIT1;                    // P1.1设为输入(可省略)
    P1IES |= BIT1;                          // P1.1设为下降沿中断
    P1IE  |= BIT1 ;                          // 允许P1.1中断
    P2DIR &= ~BIT1;                    // P2.1设为输入(可省略)
    P2IES |= BIT1;                          // P2.1设为下降沿中断
    P2IE  |= BIT1 ;                          // 允许P2.1中断
}
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    P1_IODect();                                //检测通过，则会调用事件处理函数
    P1IFG = 0;                                    //退出中断前必须手动清除IO口中断标志
}
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    P2_IODect();
    P2IFG = 0;
}
void P1_IODect()
{
    unsigned int Push_Key=0;

    Push_Key=P1IFG&(~P1DIR);
     __delay_cycles(10000);

    if((P1IN&Push_Key)==0)
   {
        switch(Push_Key)
        {
            case BIT1:    P11_Onclick();      break;
            default:                        break;      //任何情况下均加上default
        }
   }
}
void P2_IODect()
{
    unsigned int Push_Key=0;

    Push_Key=P2IFG&(~P2DIR);
     __delay_cycles(10000);

    if((P2IN&Push_Key)==0)
   {
        switch(Push_Key)
        {
            case BIT1:    P21_Onclick();      break;
            default:                        break;      //任何情况下均加上default
        }
   }
}
void P11_Onclick()
{
    flag = 2;
}
void P21_Onclick()
{
    flag = 0;
}


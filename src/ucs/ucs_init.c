/*
 * ucs_init.c
 *
 *  Created on: 2016-6-29
 *      Author: redchenjs
 *
 *		默认时钟设置：
 *			ACLK:XT1CLK(若无效低频时切换到REFO，高频切换到DCO)
 *			SMCLK:DCOCLKDIV
 *			MCLK:DCOCLKDIV
 *			FLLREFCLK:XT1CLK(若无效切换到REFO)
 *
 *		时钟信号输出口：
 *			ACLK:P1.0
 *			SMCLK:P2.2
 *			MCLK:P7.7
 *
 * 		当前信号源设置：
 *			XT1CLK:32.768kHz
 *			XT2CLK:4MHz
 *			VLOCLK:10kHz
 *			REFOCLK:32.768kHz
 *			DCOCLK:15.99MHz
 *
 *		当前时钟源设置：
 *			ACLK:XT1CLK(32.768kHz)
 *			SMCLK:XT2CLK(4MHz)
 *			MCLK:DCOCLK(15.99MHz)
 *
 */
#include "msp430.h"

#define SET_ACLK_OUT	P1SEL |= BIT0; P1DIR |= BIT0	//设置P1.0为ACLK输出
#define SET_SMCLK_OUT	P2SEL |= BIT2; P2DIR |= BIT2	//设置P2.2为SMCLK输出
#define SET_MCLK_OUT	P7SEL |= BIT7; P7DIR |= BIT7	//设置P7.7为MCLK输出
#define SET_XT1_IN		P5SEL |= BIT4 | BIT5			//设置P5.4,P5.5为XT1外部晶振输入
#define SET_XT2_IN		P5SEL |= BIT2 | BIT3			//设置P5.2,P5.3为XT2外部晶振输入
#define SET_XT1_ON		UCSCTL6 |= XCAP_3; UCSCTL6 &=~XT1OFF;//启动XT1振荡器
#define SET_XT2_ON		UCSCTL6 &=~XT2OFF;				//启动XT2振荡器
#define SET_CLK_SOURCE	UCSCTL4=(UCSCTL4&(~(SELA_7|SELS_7|SELM_7)))|SELA_0|SELS__XT2CLK|SELM_3
														//设置时钟源
//void WaitForOscillation(void)
//{
//	while (SFRIFG1 & OFIFG) {
//		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);		//UCS控制寄存器配置：清除时钟故障标志位
//		SFRIFG1 &= ~OFIFG;								//特殊功能寄存器配置：清除时钟故障标志位
//	}
//}

void SetVcoreUp (unsigned int level)
{
  // Open PMM registers for write
  PMMCTL0_H = PMMPW_H;
  // Set SVS/SVM high side new level
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
  // Set SVM low side to new level
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
  // Wait till SVM is settled
  while ((PMMIFG & SVSMLDLYIFG) == 0);
  // Clear already set flags
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;
  // Wait till new level reached
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
  // Set SVS/SVM low side to new level
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
  // Lock PMM registers for write access
  PMMCTL0_H = 0x00;
}

void UCS_Init(void)
{
//	SET_MCLK_OUT;				//开启MCLK输出
//	SET_ACLK_OUT;				//开启ACLK输出
//	SET_SMCLK_OUT;				//开启SMCLK输出
//
//	SET_XT1_IN;					//设置XT1晶振输入
//	SET_XT1_ON;					//启动XT1外部晶振
//
//	SET_XT2_IN;					//设置XT2晶振输入
//	SET_XT2_ON;					//启动XT2外部晶振
//
//	__bis_SR_register(SCG0);	//配置DCOCLK之前先设置SCG0寄存器位：MCLK停止运行
//
//	UCSCTL0 = 0x00;   			//先清零，FLL运行时，该寄存器系统会自动配置，不用管
//	UCSCTL1 = DCORSEL_6;		//调节范围约为 ~ MHZ（设置DCO的频率范围，之后设置的DCO时钟要在这个范围内，否则会出错）
//	UCSCTL2 = FLLD_1 | 243;		//FLLD=1,FLLN=243,则频率为2*（243+1）*32.768=15.99MHZ
//								//DCOCLK = D*(N+1)*(REFCLK/n)
//								//DCOCLKDIV = (N+1)*(REFCLK/n)
//
//	__bic_SR_register(SCG0);	//DCOCLK配置完毕，清除SCG0寄存器位：MCLK恢复运行
//	__delay_cycles(782000);		//延时
//
//	WaitForOscillation();		//等待时钟起振
//
//	SET_CLK_SOURCE;				//配置时钟源

//     Increase Vcore setting to level3 to support fsystem=25MHz
//     NOTE: Change core voltage one level at a time..
    SetVcoreUp (0x01);
    SetVcoreUp (0x02);
    SetVcoreUp (0x03);

    UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
    UCSCTL2 = FLLD_0 + 762;                   // Set DCO Multiplier for 25MHz
                                              // (N + 1) * FLLRef = Fdco
                                              // (762 + 1) * 32768 = 25MHz
                                              // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
    __delay_cycles(782000);

    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
    do
    {
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                              // Clear XT2,XT1,DCO fault flags
      SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

}


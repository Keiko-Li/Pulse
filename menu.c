
/*
 * menu.c
 *
 *  Created on: 2018年3月9日
 *      Author: laceyli
 */
#include <msp430.h>
#include "inc/user/System.h"

unsigned char display = 0;
unsigned char flag = 0;

void menu()
{
    if(0==flag)
    {
        ssd1331_draw_bitmap(20,16,c_chlogo,56,32,GREEN);
        __delay_cycles(25000000);
        ssd1331_draw_bitmap(20,16,c_chlogo,56,32,BLACK);
        flag = 1;
    }
    else if(1==flag)
    {
        ssd1331_display_string(0,0,"Please put your fingers to start!",FONT_1608,BLUE);
    }
    else
    {
        ssd1331_display_string(0,0,"Please put your fingers to start!",FONT_1608,BLACK);
        max30102_init();
        display = Pulse_Measure();

        while(flag)
        {
            ssd1331_display_string(16,4,"Heart Rate",FONT_1206,BLUE);
            ssd1331_display_num(16, 24, display,5, FONT_1608, BLUE);
        }
        ssd1331_clear_screen(BLACK);
    }
}


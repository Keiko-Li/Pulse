/*
 * algorithm.c
 *
 *  Created on: 2018年3月11日
 *      Author: laceyli
 */
#include <msp430.h>
#include <string.h>
#include "inc/user/System.h"

unsigned char Pulse_Algorithm(unsigned char *data)
{
    int i;
    unsigned char sum,average,count=0;
    sum = 0;
    for(i=0;i<19;i++)
    {
        if(data[i] > 40 && data[i] < 200)
        {
            sum += data[i];
            count++;
        }
    }
    average = sum / count;

    return average;
}

unsigned char Pulse_Measure(void)
{
    unsigned char a[19] = {'\0'};
    unsigned char pulse = 0;
    long sum = 0;
    int i;

    memset(a,0,sizeof(a));
    for(i=0;i<31;i++)
    {
        ssd1331_draw_bitmap(32,8,c_chheart816,32,32,BLUE);
        while(pulse < 40 || pulse > 200)
        {
            max30102_FIFO_Read(0x07,a);
            pulse = Pulse_Algorithm(a);
        }
        sum += pulse;
        pulse = 0;
        __delay_cycles(3000000);
        ssd1331_display_string(8,44,"Please wait...",FONT_1206,BLUE);
        ssd1331_draw_bitmap(32,8,c_chheart816,32,32,BLACK);
        __delay_cycles(3000000);
    }
    ssd1331_display_string(8,44,"Please wait...",FONT_1206,BLACK);
    pulse = sum / i;
    memset(a,0,sizeof(a));
    return pulse;
}



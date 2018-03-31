/*
 * System.c
 *
 *  Created on: 2018年3月9日
 *      Author: laceyli
 */
#include "inc/user/System.h"
void System_init()
{
    UCS_Init();
    i2c_init();
    spi_init();
    ssd1331_init();
    GPIO_init();
}

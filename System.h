/*
 * System.h
 *
 *  Created on: 2018年3月9日
 *      Author: laceyli
 */

#ifndef INC_USER_SYSTEM_H_
#define INC_USER_SYSTEM_H_

#include "inc/agreement/i2c.h"
#include "inc/agreement/spi.h"
#include "inc/agreement/uart.h"
#include "inc/display/fonts.h"
#include "inc/display/lcd12864.h"
#include "inc/display/ssd1306.h"
#include "inc/display/ssd1331.h"
#include "inc/tasks/max30102.h"
#include "inc/tasks/algorithm.h"
#include "inc/tasks/button.h"
#include "inc/ucs/ucs_init.h"
#include "inc/user/menu.h"

extern void System_init();

#endif /* INC_USER_SYSTEM_H_ */

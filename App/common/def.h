/*
 * def.h
 *
 *  Created on: Sep 8, 2025
 *      Author: RCY
 */

#ifndef COMMON_DEF_H_
#define COMMON_DEF_H_



#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"






#define _LED_MAX_CH		3




#define _DEF_CH_1		0
#define _DEF_CH_2		1
#define _DEF_CH_3		2
#define _DEF_CH_4		3
#define _DEF_CH_5		4
#define _DEF_CH_6		5
#define _DEF_CH_7		6


void delay_ms(uint32_t ms);
uint32_t millis(void);


#endif /* COMMON_DEF_H_ */

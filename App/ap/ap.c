/*
 * ap.c
 *
 *  Created on: Sep 8, 2025
 *      Author: RCY
 */


#include "ap.h"



void ap_init(void)
{
	led_on(_DEF_CH_1);
}


void ap_main(void)
{
	while(1)
	{
		led_toggle(_DEF_CH_1);
		delay_ms(500);
	}
}

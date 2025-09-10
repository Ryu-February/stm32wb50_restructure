/*
 * led.c
 *
 *  Created on: Sep 9, 2025
 *      Author: RCY
 */

#include "led.h"


typedef struct
{
	GPIO_TypeDef 	*port;
	uint16_t		pin;
	GPIO_PinState 	on_state;
	GPIO_PinState 	off_state;
}led_table_t;

const led_table_t led[LED_MAX_CH] =
{
	{GPIOB, GPIO_PIN_1, GPIO_PIN_SET, GPIO_PIN_RESET},	//pull-up resistor
};



void led_init(void)
{
	led_on(_DEF_CH_1);
}

void led_on(uint8_t ch)
{
	if(ch >= LED_MAX_CH)
		return;

	switch (ch)
	{
		case _DEF_CH_1:
			HAL_GPIO_WritePin(led[ch].port, led[ch].pin, led[ch].on_state);
			break;
	}
}

void led_off(uint8_t ch)
{
	if(ch >= LED_MAX_CH)
		return;

	switch (ch)
	{
		case _DEF_CH_1:
			HAL_GPIO_WritePin(led[ch].port, led[ch].pin, led[ch].off_state);
			break;
	}
}

void led_toggle(uint8_t ch)
{
	if(ch >= LED_MAX_CH)
		return;

	switch (ch)
	{
		case _DEF_CH_1 :
			HAL_GPIO_TogglePin(led[ch].port, led[ch].pin);
			break;
	}
}

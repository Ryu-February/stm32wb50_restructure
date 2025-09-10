/*
 * rgb.h
 *
 *  Created on: Sep 9, 2025
 *      Author: RCY
 */

#ifndef RGB_RGB_H_
#define RGB_RGB_H_


#include "def.h"



#define RGB_CH1_R		GPIO_PIN_5		//GPIO_A
#define RGB_CH1_G		GPIO_PIN_6
#define RGB_CH1_B		GPIO_PIN_4


typedef enum
{
	COLOR_RED = 0,
	COLOR_ORANGE,
	COLOR_YELLOW,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_PURPLE,
	COLOR_LIGHT_GREEN,
	COLOR_SKY_BLUE,
	COLOR_PINK,
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_GRAY,
	COLOR_COUNT
} color_t;


typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}rgb_led_t;


void rgb_init(void);
void rgb_set_color(color_t color);
void rgb_set_pwm(uint8_t r, uint8_t g, uint8_t b);



#endif /* RGB_RGB_H_ */

/*
 * rgb.c
 *
 *  Created on: Sep 9, 2025
 *      Author: RCY
 */


#include "rgb.h"


const rgb_led_t led_map[COLOR_COUNT] =
{
		[COLOR_RED]         = { 255,   0,   0 },
		[COLOR_ORANGE]      = { 255, 165,   0 },
		[COLOR_YELLOW]      = { 255, 255,   0 },
		[COLOR_GREEN]       = {   0, 255,   0 },
		[COLOR_BLUE]        = {   0,   0, 255 },
		[COLOR_PURPLE]      = { 160,  32, 240 },  // or VIOLET
		[COLOR_LIGHT_GREEN] = {  26, 255,  26 },
		[COLOR_SKY_BLUE]    = {  70, 200, 255 },  // LIGHT_BLUE or CYAN mix
		[COLOR_PINK]        = { 255, 105, 180 },
		[COLOR_BLACK]       = {   0,   0,   0 },
		[COLOR_WHITE]       = { 255, 255, 255 },
		[COLOR_GRAY]        = { 128, 128, 128 }
};


void rgb_init(void)
{
	HAL_GPIO_WritePin(GPIOA, RGB_CH1_R | RGB_CH1_G | RGB_CH1_B, GPIO_PIN_SET);
}

void rgb_set_color(color_t color)
{
	if(color >= COLOR_COUNT)
		return;

	rgb_set_pwm(led_map[color].r, led_map[color].g, led_map[color].b);
}

void rgb_set_pwm(uint8_t r, uint8_t g, uint8_t b)
{
	static uint8_t pwm_period = 0;

	if(++pwm_period > 255)
		pwm_period = 0;

	if(pwm_period > 255 - r)
	{
		HAL_GPIO_WritePin(GPIOA, RGB_CH1_R, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, RGB_CH1_R, GPIO_PIN_SET);
	}

	if(pwm_period > 255 - g)
	{
		HAL_GPIO_WritePin(GPIOA, RGB_CH1_G, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, RGB_CH1_G, GPIO_PIN_SET);
	}

	if(pwm_period > 255 - b)
	{
		HAL_GPIO_WritePin(GPIOA, RGB_CH1_B, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, RGB_CH1_B, GPIO_PIN_SET);
	}
}

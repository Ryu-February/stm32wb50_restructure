/*
 * ap_isr.c
 *
 *  Created on: Jul 28, 2025
 *      Author: fbcks
 */


#include "ap_isr.h"
#include "rgb.h"
#include "color.h"
#include "input.h"

volatile uint32_t timer17_ms;
volatile bool check_color;

extern volatile uint8_t detected_color;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
		case GPIO_PIN_0:
			ap_exti0_callback();
			break;
	}
}

void ap_exti0_callback(void)
{
	bool level = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);  // Pull-up 기준
	input_exti_triggered(INPUT_MODE, level);

	if(level)
		check_color = true;
}


void ap_tim2_callback(void)
{

}

void ap_tim16_callback(void)
{
	rgb_set_color(detected_color);
}

void ap_tim17_callback(void)
{
	timer17_ms++;

	input_update();


}

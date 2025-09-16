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
#include "stepper.h"

volatile uint32_t timer17_ms;
volatile bool check_color;

extern volatile uint8_t detected_color;
volatile bool stepper_enable_evt = false;


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

	if(detected_color == COLOR_BLACK)
		return;

	if(stepper_enable_evt != true)
		return;

	switch (detected_color)
	{
		case COLOR_RED :
			step_drive(OP_REVERSE);
			break;
		case COLOR_YELLOW :
			step_drive(OP_TURN_RIGHT);
			break;
		case COLOR_GREEN :
			step_drive(OP_FORWARD);
			break;
		case COLOR_BLUE :
			step_drive(OP_TURN_LEFT);
			break;
		default :
			step_drive(OP_NONE);
			break;
	}

	step_tick_isr();

	if(get_current_steps() >= 1200)
	{
		step_coast_stop();
		step_idx_init();
		odometry_steps_init();
		detected_color = COLOR_BLACK;
		stepper_enable_evt = false;
	}


}

void ap_tim17_callback(void)
{
	timer17_ms++;

	input_update();


}

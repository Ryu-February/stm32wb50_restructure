/*
 * ap.c
 *
 *  Created on: Sep 8, 2025
 *      Author: RCY
 */


#include "ap.h"


extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

volatile uint8_t detected_color = COLOR_BLACK;
volatile bool color_calibration = false;
extern volatile bool check_color;

static bool init_printed = false;
static uint8_t color_seq = 0;

static void ap_task_color_calibration(void);
static void ap_task_color_detection(void);


void ap_init(void)
{
	i2c_init();
	uart_init();

	led_init();
	rgb_init();
	color_init();
//	step_motor_init();
	step_init_all();

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim16);
	HAL_TIM_Base_Start_IT(&htim17);

	load_color_reference_table();
	debug_print_color_reference_table();
}



void ap_main(void)
{
	while(1)
	{
		if(!color_calibration && input_is_long_pressed(INPUT_MODE))
		{
			color_calibration = true;
			color_seq = 0;
			init_printed = false;
			uart_printf("[INFO] Entering color calibration mode...\r\n");
		}

		if (color_calibration)
		{
			ap_task_color_calibration();
		}
		else
		{
			ap_task_color_detection();
		}
	}
}



static void ap_task_color_calibration(void)
{
	if(!check_color) return;

	if (!init_printed)
	{
		uart_printf("-------------COLOR SETTING-------------\r\n");
		init_printed = true;
		flash_erase_color_table(BH1745_ADDR_LEFT);
		flash_erase_color_table(BH1745_ADDR_RIGHT);
	}

	if (input_is_short_pressed(INPUT_MODE))
	{
		uart_printf("color set: [%s]\r\n", color_to_string(color_seq));

		bh1745_color_data_t left  = bh1745_read_rgbc(BH1745_ADDR_LEFT);
		bh1745_color_data_t right = bh1745_read_rgbc(BH1745_ADDR_RIGHT);

		uart_printf("[LEFT]  R:%u G:%u B:%u C:%u\r\n",
					left.red, left.green, left.blue, left.clear);

		uart_printf("[RIGHT] R:%u G:%u B:%u C:%u\r\n",
					right.red, right.green, right.blue, right.clear);

		save_color_reference(BH1745_ADDR_LEFT,  color_seq, left.red, left.green, left.blue);
		save_color_reference(BH1745_ADDR_RIGHT, color_seq, right.red, right.green, right.blue);

		uart_printf("--------------------------------\r\n");

		if (++color_seq > COLOR_GRAY)
		{
			color_calibration = false;
			init_printed = false;
			color_seq = 0;
			uart_printf("-------color set finished-------\r\n");
			uart_printf("--------------------------------\r\n");
			load_color_reference_table();
			debug_print_color_reference_table();
		}
	}
}


// -------------------- 일반 색상 인식 루틴 --------------------
static void ap_task_color_detection(void)
{
	if (!check_color) return;

	uint8_t left  = classify_color_side(BH1745_ADDR_LEFT);
	uint8_t right = classify_color_side(BH1745_ADDR_RIGHT);

	if (left == right)
	{
		detected_color = left;
		uart_printf("cur_detected color: %s\r\n", color_to_string(left));
	}
	else
	{
		detected_color = COLOR_BLACK;
		uart_printf("The colors on both sides do not match!!\r\n");
		uart_printf("[LEFT]: %s | [RIGHT]: %s\r\n", color_to_string(left), color_to_string(right));
	}

	check_color = false;
}


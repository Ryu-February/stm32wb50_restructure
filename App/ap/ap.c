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
extern volatile bool stepper_enable_evt;

static bool init_printed = false;
static uint8_t color_seq = 0;

static color_t plan_color = COLOR_BLACK;

volatile bool plan_armed = false;    // 1초 타이머 통과했는지 표시
volatile bool after_1s_evt = false;

extern volatile uint32_t timer17_ms;

static void ap_task_color_calibration(void);
static void ap_task_color_detection(void);
static void enable_stepper_after_1s(void);
static void ap_motion_update(void);

const lt_config_t lt =
{
	.Kp = 20.f, .Ki = 0.f, .Kd = 15.f,
	.base_ticks = 1500, .min_ticks = 500, .max_ticks = 2500,
	.interval_ms = 5
};


void ap_init(void)
{
	i2c_init();
	uart_init();

	led_init();
	rgb_init();
	color_init();
//	step_motor_init();
	step_init_all();
    line_tracing_init(&lt);
    line_tracing_enable(false);

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim16);
	HAL_TIM_Base_Start_IT(&htim17);

	load_color_reference_table();
	calculate_color_brightness_offset();
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
			enable_stepper_after_1s();
			ap_motion_update();
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

static void enable_stepper_after_1s(void)
{
	if(detected_color == COLOR_BLACK)
		return;

	if(after_1s_evt == true)
		return;

	static uint32_t t = 0;
	uint32_t now = millis();

	if(t == 0)	t = now;

	if((uint32_t)now - t > 1000)
	{
		t = 0;
		stepper_enable_evt = true;
		after_1s_evt = true;
	}
}

static void ap_motion_update(void)
{
	if(detected_color != COLOR_BLACK && stepper_enable_evt && !plan_armed)
	{
		plan_armed = true;
		plan_color = (color_t)detected_color;
		motion_plan_color(plan_color);
//		stepper_enable_evt = false;
	}

	motion_service();

	if(!plan_armed)
	{

	}
	else
	{

	}

	// 보라색이면: 1초 유지 이벤트가 떴을 때 라인트레이싱 시작
	if (!line_tracing_enabled())
	{
		if (detected_color == COLOR_PURPLE)
		{
			if (stepper_enable_evt)
			{
				uart_printf("[LT] Start by PURPLE card.\r\n");
				line_tracing_enable(true);
//				stepper_enable_evt = false;   // 이벤트 소모
			}
		}
	}
	else
	{
//		uart_printf("in line-tracing mode\r\n");
		// 라인트레이싱이 이미 켜져 있을 때:
		//  - 계속 업데이트
		//  - 보라/검정이 아닌 다른 색이 1초 유지되면 라인트레이싱 종료
		line_tracing_update(timer17_ms);

		if (detected_color != COLOR_PURPLE && detected_color != COLOR_BLACK)
		{
			if (stepper_enable_evt)
			{
				uart_printf("[LT] Stop (another color card detected).\r\n");
				line_tracing_enable(false);
				stepper_enable_evt = false;
			}
		}
		// 검정이면 그대로 유지(라인 위에서 센서 상황에 따라 카드가 안 보일 수 있으니 유지)
		return;
	}
}

/*
static void ap_card_mode(void)
{

}
*/

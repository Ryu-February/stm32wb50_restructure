/*
 * input.c
 *
 *  Created on: Sep 8, 2025
 *      Author: RCY
 */


#include "input.h"


#define NUM_INPUTS 7


static bool is_pressed[NUM_INPUTS];
static uint32_t press_time[NUM_INPUTS];  // 1ms 단위 누른 시간
static bool short_press_flag[NUM_INPUTS];
static bool long_press_flag[NUM_INPUTS];





void input_exti_triggered(input_id_t id, bool level)
{
	is_pressed[id] = level;
}

void input_update(void)
{
	for(int i = 0; i < NUM_INPUTS; i++)
	{
		if(is_pressed[i])
		{
			press_time[i]++;

			if(press_time[i] == 3000)
			{
				long_press_flag[i] = true;
			}
		}
		else
		{
			if(press_time[i] > 0 && press_time[i] < 3000)
			{
				short_press_flag[i] = true;
			}
			press_time[i] = 0;
		}
	}
}

bool input_is_short_pressed(input_id_t id)
{
	bool ret = short_press_flag[id];
	short_press_flag[id] = false;
	return ret;
}

bool input_is_long_pressed(input_id_t id)
{
	bool ret = long_press_flag[id];
	long_press_flag[id] = false;
	return ret;
}

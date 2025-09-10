/*
 * ir.c
 *
 *  Created on: Sep 8, 2025
 *      Author: RCY
 */


#include "ir.h"



extern ADC_HandleTypeDef hadc1;


uint16_t ir_read_adc(void)
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	uint16_t val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return val; // 0~4095
}

uint8_t ir_is_black(void)
{
    return (ir_read_adc() <= IR_THRESHOLD) ? 1 : 0;
}

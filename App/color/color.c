/*
 * color.c
 *
 *  Created on: Jul 28, 2025
 *      Author: fbcks
 */

#include "color.h"
#include "flash.h"
#include "uart.h"
//#include "step.h"
#include "i2c.h"


reference_entry_t color_reference_tbl_left[COLOR_COUNT];
reference_entry_t color_reference_tbl_right[COLOR_COUNT];

extern uint8_t offset_side;
extern uint16_t offset_black;
extern uint16_t offset_white;
extern uint16_t offset_average;


color_mode_t insert_queue[MAX_INSERTED_COMMANDS];
uint8_t insert_index = 0;



void bh1745_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data)
{
    i2c_write(dev_addr, reg, data);
}

void bh1745_init(uint8_t dev_addr)
{
    // 1. SW Reset
    bh1745_write_reg(dev_addr, 0x40, 0x80);  // SYSTEM_CONTROL: Software Reset
    HAL_Delay(10);

    // 2. Measurement Time 설정 (MODE_CONTROL1)
    bh1745_write_reg(dev_addr, 0x41, 0x00);  // 160ms

    // 3. Gain 설정 + RGBC Enable (MODE_CONTROL2)
    bh1745_write_reg(dev_addr, 0x42, 0x12);  // GAIN = 1x, Bit4(RGBC_EN) = 1

    // 4. RGB 측정 트리거 (MODE_CONTROL3)
    bh1745_write_reg(dev_addr, 0x44, 0x02);  // RGB measurement start
}

void color_init(void)
{
	bh1745_init(BH1745_ADDR_LEFT);
	bh1745_init(BH1745_ADDR_RIGHT);
}

uint16_t bh1745_read_u16(uint8_t dev_addr, uint8_t lsb_reg)
{
    uint8_t lsb = i2c_read(dev_addr, lsb_reg);
    uint8_t msb = i2c_read(dev_addr, lsb_reg + 1);
    return (msb << 8) | lsb;
}

bh1745_color_data_t bh1745_read_rgbc(uint8_t dev_addr)
{
    bh1745_color_data_t color;

    color.red   = bh1745_read_u16(dev_addr, 0x50);
    color.green = bh1745_read_u16(dev_addr, 0x52);
    color.blue  = bh1745_read_u16(dev_addr, 0x54);
    color.clear = bh1745_read_u16(dev_addr, 0x56);

    return color;
}


void save_color_reference(uint8_t sensor_side, color_t color, uint16_t r, uint16_t g, uint16_t b)
{
	rgb_raw_t	raw		= {.red_raw = r, .green_raw = g, .blue_raw = b};
    uint64_t 	offset	= calculate_brightness(r, g, b);

    reference_entry_t entry = { .raw = raw, .color = color, .offset = offset };

    if (sensor_side == BH1745_ADDR_LEFT)
    {
        color_reference_tbl_left[color] = entry;
    }
    else
    {
        color_reference_tbl_right[color] = entry;
    }

    // Flash에 저장!
    flash_write_color_reference(sensor_side, color, entry);
}


color_t classify_color(uint8_t left_right, uint16_t r, uint16_t g, uint16_t b, uint16_t c)
{
	float min_dist = 1e9;
	color_t best_match = COLOR_GRAY;

    const reference_entry_t* table;
    int table_size = COLOR_COUNT;

    if (left_right == BH1745_ADDR_LEFT)  // LEFT
    {
        table = color_reference_tbl_left;
    }
    else // RIGHT
    {
        table = color_reference_tbl_right;
    }

    for (int i = 0; i < table_size; i++)
    {
    	float dr = (float)r - table[i].raw.red_raw;
		float dg = (float)g - table[i].raw.green_raw;
		float db = (float)b - table[i].raw.blue_raw;

        float dist = dr * dr + dg * dg + db * db;

        if (dist < min_dist)
        {
            min_dist = dist;
            best_match = table[i].color;
        }
    }

	return best_match;
}

uint8_t classify_color_side(uint8_t color_side)
{
	uint8_t addr = color_side;

	bh1745_color_data_t color_rgbc;
	color_t detected = COLOR_BLACK;

	color_rgbc 	= bh1745_read_rgbc(addr);

	detected =
			classify_color(addr, color_rgbc.red, color_rgbc.green, color_rgbc.blue, color_rgbc.clear);

	return (uint8_t) detected;
}

const char* color_to_string(color_t color)
{
    static const char* color_names[] =
    {
        "RED",
        "ORANGE",
        "YELLOW",
        "GREEN",
        "BLUE",
        "PURPLE",
        "LIGHT_GREEN",
        "SKY_BLUE",
        "PINK",
        "BLACK",
        "WHITE",
        "GRAY"
    };

    if (color < 0 || color >= COLOR_COUNT)
        return "UNKNOWN";

    return color_names[color];
}

void load_color_reference_table(void)
{
    for (int i = 0; i < COLOR_COUNT; i++)
    {
        color_reference_tbl_left[i] = flash_read_color_reference(BH1745_ADDR_LEFT, i);
        color_reference_tbl_right[i] = flash_read_color_reference(BH1745_ADDR_RIGHT, i);
    }
}

void debug_print_color_reference_table(void)
{
    uart_printf("=== LEFT COLOR REFERENCE TABLE ===\r\n");
    for (int i = 0; i < COLOR_COUNT; i++)
    {
        reference_entry_t e = color_reference_tbl_left[i];
        uart_printf("[%2d | %-11s] R: %4d, G: %4d, B: %4d, OFFSET: %8llu\r\n",
                    i, color_to_string(e.color),
                    e.raw.red_raw, e.raw.green_raw, e.raw.blue_raw, e.offset);
    }

    uart_printf("=== RIGHT COLOR REFERENCE TABLE ===\r\n");
    for (int i = 0; i < COLOR_COUNT; i++)
    {
        reference_entry_t e = color_reference_tbl_right[i];
        uart_printf("[%2d | %-11s] R: %4d, G: %4d, B: %4d, OFFSET: %8llu\r\n",
                    i, color_to_string(e.color),
                    e.raw.red_raw, e.raw.green_raw, e.raw.blue_raw, e.offset);
    }
    uart_printf("=== BRIGHTNESS OFFSET TABLE ===\r\n");
//    uart_printf("offset_black: %d | offset_white: %d\r\n", offset_black, offset_white);
//	uart_printf("offset_aver: %d\r\n", offset_average);
}

uint32_t calculate_brightness(uint16_t r, uint16_t g, uint16_t b)
{
//    return 0.2126f * r + 0.7152f * g + 0.0722f * b;

    return (218 * r + 732 * g + 74 * b) >> 10;
}

void calculate_color_brightness_offset(void)
{
    offset_black = abs(color_reference_tbl_left[COLOR_BLACK].offset - color_reference_tbl_right[COLOR_BLACK].offset);
    offset_white = abs(color_reference_tbl_left[COLOR_WHITE].offset - color_reference_tbl_right[COLOR_WHITE].offset);

    offset_side = (color_reference_tbl_left[COLOR_BLACK].offset > color_reference_tbl_right[COLOR_BLACK].offset)
    					? LEFT : RIGHT;
    offset_average = (offset_black + offset_white) / 2;
}

color_mode_t color_to_mode(color_t color)
{
	switch (color)
	{
		case COLOR_RED:         return MODE_FORWARD;
		case COLOR_ORANGE:      return MODE_BACKWARD;
		case COLOR_YELLOW:      return MODE_LEFT;
		case COLOR_GREEN:       return MODE_RIGHT;
		case COLOR_BLUE:        return MODE_LINE_TRACE;
		case COLOR_PURPLE:      return MODE_FAST_FORWARD;
		case COLOR_LIGHT_GREEN: return MODE_SLOW_FORWARD;
		case COLOR_SKY_BLUE:    return MODE_FAST_BACKWARD;
		case COLOR_PINK:        return MODE_SLOW_BACKWARD;
		case COLOR_GRAY:        return MODE_LONG_FORWARD;
		default:                return MODE_NONE;
	}
}

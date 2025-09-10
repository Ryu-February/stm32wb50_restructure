/*
 * color.h
 *
 *  Created on: Jul 28, 2025
 *      Author: fbcks
 */

#ifndef COLOR_COLOR_H_
#define COLOR_COLOR_H_


#include "def.h"
#include "rgb.h"

#define MODE_CALIBRATION	1

#define BH1745_ADDR_LEFT        	0x38 // 7bit << 1
#define BH1745_ADDR_RIGHT      		0x39 // 7bit << 1

// Register Addresses
#define BH1745_REG_MODE_CTRL1   	0x41
#define BH1745_REG_MODE_CTRL2   	0x42
#define BH1745_REG_MODE_CTRL3   	0x44
#define BH1745_REG_RED_DATA_LSB 	0x50
#define BH1745_REG_GREEN_DATA_LSB 	0x52
#define BH1745_REG_BLUE_DATA_LSB 	0x54
#define BH1745_REG_CLEAR_DATA_LSB 	0x56

#define BH1745_I2C_ADDR         	(0x38 << 1) // 0x70
#define BH1745_REG_MANUFACTURER_ID  0x92

#define MAX_INSERTED_COMMANDS 		20

typedef struct
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t clear;
} bh1745_color_data_t;


typedef struct {
    uint16_t red_raw;
    uint16_t green_raw;
    uint16_t blue_raw;
} rgb_raw_t;

typedef struct /*__attribute__((packed, aligned(8)))*/{
	rgb_raw_t raw;
    color_t color;
    uint8_t  _pad[1];    	// ✅ 여기까지가 정확히 8바이트(패딩용)
    uint64_t offset;		//flash 메모리에 저장할 때 8바이트 단위로 맞춰야 해서 uint64_t로 구조체 총합 16바이트로 맞춤
} reference_entry_t;

//_Static_assert(sizeof(reference_entry_t) == 16, "reference_entry_t must be 16 bytes");

typedef enum
{
	MODE_NONE = 0,           // 기본값 또는 동작 없음
	MODE_FORWARD,            // 빨강 → 전진
	MODE_BACKWARD,           // 주황 → 후진
	MODE_LEFT,               // 노랑 → 좌회전
	MODE_RIGHT,              // 초록 → 우회전
	MODE_LINE_TRACE,         // 파랑 → 라인트레이싱
	MODE_FAST_FORWARD,       // 보라 → 빠른 전진 (마이크로스텝)
	MODE_SLOW_FORWARD,       // 연두 → 느린 전진 (마이크로스텝)
	MODE_FAST_BACKWARD,      // 하늘 → 빠른 후진 (마이크로스텝)
	MODE_SLOW_BACKWARD,      // 분홍 → 느린 후진 (마이크로스텝)
	MODE_LONG_FORWARD,       // 회색 → 길게 전진
	MODE_INSERT,
	MODE_RUN,
	MODE_REPEAT_TWICE,
	MODE_REPEAT_THRICE,
	MODE_COUNT
} color_mode_t;





void bh1745_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);
void bh1745_init(uint8_t dev_addr);
void color_init(void);

uint16_t bh1745_read_u16(uint8_t dev_addr, uint8_t lsb_reg);
bh1745_color_data_t bh1745_read_rgbc(uint8_t dev_addr);
bh1745_color_data_t bh1745_read_rgbc(uint8_t dev_addr);

void save_color_reference(uint8_t sensor_side, color_t color, uint16_t r, uint16_t g, uint16_t b);
//rgb_ratio_t get_rgb_ratio(uint16_t r, uint16_t g, uint16_t b);
color_t classify_color(uint8_t left_right, uint16_t r, uint16_t g, uint16_t b, uint16_t c);
uint8_t classify_color_side(uint8_t color_side);

const char* color_to_string(color_t color);
void load_color_reference_table(void);
void debug_print_color_reference_table(void);
uint32_t calculate_brightness(uint16_t r, uint16_t g, uint16_t b);
void calculate_color_brightness_offset(void);
color_mode_t color_to_mode(color_t color);


#endif /* COLOR_COLOR_H_ */

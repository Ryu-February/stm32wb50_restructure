/*
 * stepper.c
 *
 *  Created on: Sep 10, 2025
 *      Author: RCY
 */


#include "stepper.h"

#include "uart.h"


// ---- External timers (provide from your BSP) ----
// TIM_PWM_CNT: ARR must be 255; we only read CNT (0..255) for software PWM compare
// TIM_TICK : free‑running tick for step period (e.g., 1us per tick)
extern TIM_HandleTypeDef htim2; // use as PWM CNT (ARR=255)
extern TIM_HandleTypeDef htim16; // use as tick source (1us CNT)




#define TIM_PWM_CNT ((uint32_t)(TIM2->CNT))
#define TIM_TICK_CNT (read_tick32())

static inline uint32_t read_tick32(void){ return TIM2->CNT; }


// ---- Global motors ----
static StepLL left = {
	.in1p = GPIOA, .in1b = GPIO_PIN_0,
	.in2p = GPIOA, .in2b = GPIO_PIN_1,
	.in3p = GPIOA, .in3b = GPIO_PIN_2,
	.in4p = GPIOA, .in4b = GPIO_PIN_3,

	.step_idx = 0,
	.period_ticks = 1000,
	.prev_tick = 0,
	.total_step = 0,
#if (_USE_STEP_NUM == _STEP_NUM_119)
	.dir_sign = -1,
#else
	.dir_sign = +1,
#endif
};


static StepLL right = {
	.in1p = GPIOB, .in1b = GPIO_PIN_4,
	.in2p = GPIOB, .in2b = GPIO_PIN_5,
	.in3p = GPIOB, .in3b = GPIO_PIN_6,
	.in4p = GPIOB, .in4b = GPIO_PIN_7,

	.step_idx = 0,
	.period_ticks = 1000,
	.prev_tick = 0,
	.total_step = 0,
	#if (_USE_STEP_NUM == _STEP_NUM_119)
	.dir_sign = +1,
	#else
	.dir_sign = -1,
	#endif
};


// ---- LUTs ----
#if (_USE_STEP_MODE == _STEP_MODE_MICRO)
	const uint8_t step_table[32] = {			//sin(degree) -> pwm
	  128, 152, 176, 198, 218, 234, 245, 253,
	  255, 253, 245, 234, 218, 198, 176, 152,
	  128, 103,  79,  57,  37,  21,  10,   2,
		0,   2,  10,  21,  37,  57,  79, 103
	};
#elif (_USE_STEP_MODE == _STEP_MODE_FULL)
	static const uint8_t step_table[4][4] = {
		{1,0,1,0},
		{0,1,1,0},
		{0,1,0,1},
		{1,0,0,1}
	};
#else // HALF
	static const uint8_t step_table[8][4] = {
		{1,0,1,0},
		{0,0,1,0},
		{0,1,1,0},
		{0,1,0,0},
		{0,1,0,1},
		{0,0,0,1},
		{1,0,0,1},
		{1,0,0,0}
	};
#endif


// ---- Helpers ----
static inline uint32_t diff_u32(uint32_t a, uint32_t b)
{
	return (uint32_t)(a-b);
}


static inline void gpio_pwm4(
	GPIO_TypeDef* p1, uint16_t b1,
	GPIO_TypeDef* p2, uint16_t b2,
	GPIO_TypeDef* p3, uint16_t b3,
	GPIO_TypeDef* p4, uint16_t b4,
	uint8_t now, uint8_t vA, uint8_t vB)
{
	uint32_t set1 = (now < vA) ? b1 : 0, rst1 = (now < vA) ? 0 : ((uint32_t)b1 << 16);
	uint32_t set2 = (now < (uint8_t)(255 - vA)) ? b2 : 0, rst2 = (now < (uint8_t)(255 - vA)) ? 0 : ((uint32_t)b2 << 16);
	uint32_t set3 = (now < vB) ? b3 : 0, rst3 = (now < vB) ? 0 : ((uint32_t)b3 << 16);
	uint32_t set4 = (now < (uint8_t)(255 - vB)) ? b4 : 0, rst4 = (now < (uint8_t)(255 - vB)) ? 0 : ((uint32_t)b4 << 16);

	p1->BSRR = set1 | rst1;
	p2->BSRR = set2 | rst2;
	p3->BSRR = set3 | rst3;
	p4->BSRR = set4 | rst4;
}


static inline void apply_pwm_micro(StepLL* m, uint8_t now)
{
#if (_USE_STEP_MODE == _STEP_MODE_MICRO)
	uint8_t vA = step_table[m->step_idx & STEP_MASK];
	uint8_t vB = step_table[(m->step_idx + (STEP_TABLE_SIZE >> 2)) & STEP_MASK];

	gpio_pwm4(m->in1p, m->in1b, m->in2p, m->in2b, m->in3p, m->in3b, m->in4p, m->in4b, now, vA, vB);
#else
	(void)now; // silent
#endif
}


static inline void apply_coils_table(StepLL* m)
{
#if (_USE_STEP_MODE != _STEP_MODE_MICRO)
	const uint8_t* s = step_table[m->step_idx & STEP_MASK];
	// Map to BSRR writes (set or reset)
	uint32_t set1 = s[0]? m->in1b:0, rst1 = s[0]?0:((uint32_t)m->in1b<<16);
	uint32_t set2 = s[1]? m->in2b:0, rst2 = s[1]?0:((uint32_t)m->in2b<<16);
	uint32_t set3 = s[2]? m->in3b:0, rst3 = s[2]?0:((uint32_t)m->in3b<<16);
	uint32_t set4 = s[3]? m->in4b:0, rst4 = s[3]?0:((uint32_t)m->in4b<<16);
	m->in1p->BSRR = set1 | rst1;
	m->in2p->BSRR = set2 | rst2;
	m->in3p->BSRR = set3 | rst3;
	m->in4p->BSRR = set4 | rst4;
#endif
}


static inline void try_advance(StepLL* m, uint32_t now_tick)
{
	if (diff_u32(now_tick, m->prev_tick) >= m->period_ticks)
	{
		m->prev_tick = now_tick;
		m->total_step++;
		m->step_idx = (uint16_t)((m->step_idx + m->dir_sign) & STEP_MASK);
	}
}

// ---- Public Impl ----
void step_init_all(void)
{
	left.step_idx = right.step_idx = 0;
	left.prev_tick = right.prev_tick = read_tick32();
	left.total_step = right.total_step = 0;
}


void step_idx_init(void)
{
	left.step_idx = right.step_idx = 0;
}


void step_tick_isr(void)
{
	// Call this from your actual ISR (e.g., TIM16 IRQ) at ~10us period
	uint8_t pwm_now = (uint8_t)TIM_PWM_CNT; // (uint8_t) -> 'pwm_now' must be 0-255
	uint32_t tick_now = read_tick32(); // free‑running tick (wraps) | tim2


	#if (_USE_STEP_MODE == _STEP_MODE_MICRO)
	apply_pwm_micro(&left, pwm_now);
	apply_pwm_micro(&right, pwm_now);
	#else
	apply_coils_table(&left);
	apply_coils_table(&right);
	#endif
	try_advance(&left, tick_now);
	try_advance(&right, tick_now);
}


void step_set_period_ticks(uint32_t left_ticks, uint32_t right_ticks)
{
	left.period_ticks = left_ticks ? left_ticks : 1;
	right.period_ticks = right_ticks ? right_ticks : 1;
}


void step_set_dir(int8_t left_sign, int8_t right_sign)
{
	left.dir_sign = (left_sign >= 0) ? +1 : -1;
	right.dir_sign = (right_sign >= 0) ? +1 : -1;
}


void step_stop(void)
{
	// brake = 4핀 모두 High (A3916 보드 동작 확인 필요)
	left.in1p->BSRR = left.in1b;
	left.in2p->BSRR = left.in2b;
	left.in3p->BSRR = left.in3b;
	left.in4p->BSRR = left.in4b;


	right.in1p->BSRR = right.in1b;
	right.in2p->BSRR = right.in2b;
	right.in3p->BSRR = right.in3b;
	right.in4p->BSRR = right.in4b;
}


uint32_t get_current_steps(void)
{
	uint32_t l = left.total_step; // single 32b read is atomic on M4
	uint32_t r = right.total_step;
	return (l + r) / 2u;
}


void total_step_init(void)
{
	left.total_step = right.total_step = 0;
}


// ---- Compatibility helpers ----
void step_drive(StepOperation op)
{
	switch(op)
	{
		case OP_FORWARD:
			step_set_dir(+1, +1);
			break;
		case OP_REVERSE:
			step_set_dir(-1, -1);
			break;
		case OP_TURN_LEFT:
			step_set_dir(-1, +1);
			break;
		case OP_TURN_RIGHT:
			step_set_dir(+1, -1);
			break;
		case OP_STOP:
			step_stop();
			break;
		default:
			break;
	}
}


void step_drive_ratio(uint16_t left_ticks, uint16_t right_ticks)
{
	step_set_period_ticks(left_ticks, right_ticks);
}

// ---- Mappings (fixed macro bug: compare _USE_STEP_NUM, not _USE_STEP_MODE) ----
StepOperation mode_to_step(color_mode_t mode)
{
	switch(mode)
	{
		case MODE_FORWARD:
		case MODE_FAST_FORWARD:
		case MODE_SLOW_FORWARD:
		case MODE_LONG_FORWARD:
			return OP_FORWARD;

		case MODE_BACKWARD:
		case MODE_FAST_BACKWARD:
		case MODE_SLOW_BACKWARD:
			return OP_REVERSE;

		case MODE_LEFT:
			return OP_TURN_LEFT;
		case MODE_RIGHT:
			return OP_TURN_RIGHT;
		default:
			return OP_NONE;
	}
}


uint16_t mode_to_step_count(color_mode_t mode)
{
#if (_USE_STEP_NUM == _STEP_NUM_119)
	switch(mode)
	{
		case MODE_FORWARD:
		case MODE_BACKWARD:
		case MODE_FAST_FORWARD:
		case MODE_SLOW_FORWARD:
			return 1000;
		case MODE_FAST_BACKWARD:
		case MODE_SLOW_BACKWARD:
			return 850;
		case MODE_LEFT:
		case MODE_RIGHT:
			return 390;
		case MODE_LONG_FORWARD:
			return 1900;
		case MODE_LINE_TRACE:
			return 30000;
		default:
			return 0;
	}
#elif (_USE_STEP_NUM == _STEP_NUM_729)
	switch(mode)
	{
		case MODE_FORWARD:
		case MODE_BACKWARD:
		case MODE_FAST_FORWARD:
		case MODE_SLOW_FORWARD:
			return 2800;
		case MODE_FAST_BACKWARD:
		case MODE_SLOW_BACKWARD:
			return 2050;
		case MODE_LEFT:
		case MODE_RIGHT:
			return 990;
		case MODE_LONG_FORWARD:
			return 1900;
		case MODE_LINE_TRACE:
			return 30000;
		default:
			return 0;
}
#else // _STEP_NUM_728 or others -> tune as needed
	switch(mode)
	{
		default:
			return 0;
	}
#endif
}

uint16_t mode_to_left_period(color_mode_t mode){
#if (_USE_STEP_NUM == _STEP_NUM_119)
	switch(mode)
	{
		case MODE_FAST_FORWARD: return 2000;
		case MODE_SLOW_FORWARD: return 700;
		case MODE_FAST_BACKWARD: return 1500;
		case MODE_SLOW_BACKWARD: return 1000;
		default: return 1000;
	}
#elif (_USE_STEP_NUM == _STEP_NUM_729)
	switch(mode)
	{
		case MODE_FAST_FORWARD: return 2000;
		case MODE_SLOW_FORWARD: return 700;
		case MODE_FAST_BACKWARD: return 1500;
		case MODE_SLOW_BACKWARD: return 1000;
		default: return 500;
	}
#else
	return 1000;
#endif
}


uint16_t mode_to_right_period(color_mode_t mode)
{
#if (_USE_STEP_NUM == _STEP_NUM_119)
	switch(mode)
	{
		case MODE_FAST_FORWARD: return 700;
		case MODE_SLOW_FORWARD: return 2000;
		case MODE_FAST_BACKWARD: return 1000;
		case MODE_SLOW_BACKWARD: return 1500;
		default: return 500;
	}
#elif (_USE_STEP_NUM == _STEP_NUM_729)
	switch(mode)
	{
		case MODE_FAST_FORWARD: return 700;
		case MODE_SLOW_FORWARD: return 2000;
		case MODE_FAST_BACKWARD: return 1000;
		case MODE_SLOW_BACKWARD: return 1500;
		default: return 500;
	}
#else
	return 500;
#endif
}


// ---- Utils ----
uint32_t pwm_to_rpm(uint8_t pwm)
{
	if (pwm > MAX_SPEED)
		pwm = MAX_SPEED;

	return (uint32_t)pwm * SAFE_MAX_RPM / MAX_SPEED;
}


uint32_t rpm_to_period_ticks(uint16_t rpm, uint32_t tick_hz){
if (rpm == 0) rpm = 1;
if (rpm > SAFE_MAX_RPM) rpm = SAFE_MAX_RPM;
// Period (ticks per index step) = tick_hz / (steps_per_sec)
// steps_per_sec = rpm * STEP_PER_REV / 60
// => ticks = tick_hz * 60 / (rpm * STEP_PER_REV)
uint32_t num = tick_hz * 60UL;
uint32_t den = (uint32_t)rpm * (uint32_t)STEP_PER_REV;
return (den ? (num / den) : num);
}

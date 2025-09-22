/*
 * card_mode.c
 *
 *  Created on: Sep 18, 2025
 *      Author: RCY
 */


#include "card_mode.h"
#include "uart.h"
#include "stepper.h"

extern volatile bool after_1s_evt;
extern volatile bool stepper_enable_evt;
extern volatile bool plan_armed;    // 1초 타이머 통과했는지 표시

extern uint32_t millis(void);


// ===== 기존 static들에 아래 변수들 추가 =====
static uint16_t  s_gap_ms        = 1000;   // 기본 1초
static bool      s_gap_waiting   = false;
static uint32_t  s_gap_deadline  = 0;



typedef enum
{
    CM_IDLE = 0,        // 모드 아님
    CM_PROGRAMMING,     // 카드 나열해서 시퀀스 입력 중
    CM_READY_TO_RUN,    // 트리거 들어오면 실행 시작 가능
    CM_RUNNING          // 실행 중 (버퍼 FIFO로 순차 실행)
} cm_state_t;

/* 시퀀스 버퍼 */
static card_op_t  s_seq[CARD_SEQ_MAX];
static uint16_t   s_len        = 0;       // 유효 길이
static uint16_t   s_exec_idx   = 0;       // 실행 중 현재 인덱스

/* 모드/상태 */
static bool       s_active     = false;
static cm_state_t s_state      = CM_IDLE;
static bool       s_plan_ready = false;   // ap가 꺼내갈 실행 계획 준비됨 플래그
static color_t    s_plan_color = COLOR_BLACK;

/* 입력(프로그래밍) 매핑: 카드 색 -> 동작
   기본: RED->FWD, GREEN->REV, YELLOW->RIGHT, BLUE->LEFT */
static card_op_t  s_input_map[COLOR_COUNT];

/* 실행(모션) 매핑: 동작 -> motion_plan_color()에서 기대하는 색
   (프로젝트 요약에 따르면: RED=REVERSE, GREEN=FORWARD, YELLOW=RIGHT, BLUE=LEFT) */
static color_t    s_exec_map_for_plan[CARD_OP_TURN_LEFT + 1];

/* 트리거: 왼쪽=GREEN, 오른쪽=ORANGE */
static inline bool is_trigger(uint8_t left, uint8_t right)
{
    return (left == COLOR_LIGHT_GREEN) && (right == COLOR_ORANGE);
}


/* ===== 내부 유틸 ===== */

static void seq_clear(void)
{
    s_len      = 0;
    s_exec_idx = 0;
}

static bool seq_push(card_op_t op)
{
    if (s_len >= CARD_SEQ_MAX)
    {
        return false;
    }

    s_seq[s_len++] = op;
    return true;
}

static bool op_to_plan_color(card_op_t op, color_t *out)
{
    if (out == NULL)
    {
        return false;
    }

    if (op <= CARD_OP_NONE || op > CARD_OP_TURN_LEFT)
    {
        return false;
    }

    *out = s_exec_map_for_plan[op];
    return true;
}

static const char* op_to_str(card_op_t op)
{
    switch (op)
    {
        case CARD_OP_FORWARD:     return "FORWARD";
        case CARD_OP_REVERSE:     return "REVERSE";
        case CARD_OP_TURN_RIGHT:  return "TURN_RIGHT";
        case CARD_OP_TURN_LEFT:   return "TURN_LEFT";
        default:                  return "NONE";
    }
}

/* ===== 공개 API 구현 ===== */

void card_mode_init(void)
{
    /* 상태 */
    s_active     = false;
    s_state      = CM_IDLE;
    s_plan_ready = false;
    s_plan_color = COLOR_BLACK;

    s_gap_ms       = 1000;
	s_gap_waiting  = false;
	s_gap_deadline = 0;

    /* 버퍼 */
    seq_clear();

    /* 기본 입력 매핑 (네가 예시로 준 것) */
    for (int i = 0; i < COLOR_COUNT; ++i)
    {
        s_input_map[i] = CARD_OP_NONE;
    }
    s_input_map[COLOR_RED]   = CARD_OP_REVERSE;
    s_input_map[COLOR_GREEN] = CARD_OP_FORWARD;
    s_input_map[COLOR_YELLOW]= CARD_OP_TURN_RIGHT;
    s_input_map[COLOR_BLUE]  = CARD_OP_TURN_LEFT;

    /* 실행 매핑 (프로젝트 motion_plan_color() 기준) */
    for (int i = 0; i <= CARD_OP_TURN_LEFT; ++i)
    {
        s_exec_map_for_plan[i] = COLOR_BLACK;
    }
    s_exec_map_for_plan[CARD_OP_FORWARD]    = COLOR_GREEN; // 전진 실행은 GREEN 계획
    s_exec_map_for_plan[CARD_OP_REVERSE]    = COLOR_RED;   // 후진 실행은 RED 계획
    s_exec_map_for_plan[CARD_OP_TURN_RIGHT] = COLOR_YELLOW;
    s_exec_map_for_plan[CARD_OP_TURN_LEFT]  = COLOR_BLUE;
}

void card_mode_reset(void)
{
    card_mode_init();
}

bool card_mode_is_active(void)
{
    return s_active;
}

uint16_t card_mode_seq_size(void)
{
    return s_len;
}

void card_mode_seq_clear(void)
{
    seq_clear();
    uart_printf("[CARD] Sequence cleared.\r\n");
}

void card_mode_set_input_mapping(color_t color, card_op_t op)
{
    if (color < COLOR_COUNT)
    {
        s_input_map[color] = op;
    }
}

void card_mode_on_dual_colors(uint8_t left, uint8_t right)
{
    if (!is_trigger(left, right))
    {
        return;
    }

    if (!s_active)
    {
        s_active = true;
        s_state  = CM_PROGRAMMING;
        seq_clear();
        uart_printf("[CARD] Enter PROGRAMMING mode. (Trigger)\r\n");
        uart_printf("[CARD] Show cards in order (max %u). Trigger again to RUN.\r\n", CARD_SEQ_MAX);
        return;
    }

    if (s_state == CM_PROGRAMMING || s_state == CM_READY_TO_RUN)
    {
        if (s_len == 0)
        {
            uart_printf("[CARD] No sequence. Stay in PROGRAMMING.\r\n");
            return;
        }

        s_state      = CM_RUNNING;
        s_exec_idx   = 0;
        s_plan_ready = false;
        uart_printf("[CARD] Start RUN (%u steps).\r\n", s_len);

        // ★ 카드 실행은 1초 대기/색일치와 무관하게 바로 시작
        stepper_enable_evt = true;
        after_1s_evt       = true;   // 필요하면 세워두기
        return;
    }

    if (s_state == CM_RUNNING)
    {
        // 실행 중 트리거가 들어오면 무시 (혹은 Stop 기능으로 바꿀 수 있음)
        uart_printf("[CARD] Trigger ignored (RUNNING).\r\n");
    }
}

void card_mode_on_detected_color(color_t color)
{
    if (!s_active)
    {
        return;
    }

    if (s_state != CM_PROGRAMMING)
    {
        return;
    }

    if (color >= COLOR_COUNT)
    {
        return;
    }

    card_op_t op = s_input_map[color];

    if (op == CARD_OP_NONE)
    {
        // 입력용으로 쓰지 않는 색은 무시(예: 잡색/회색 등)
        return;
    }

    if (!seq_push(op))
    {
        uart_printf("[CARD] Sequence full (%u). Consider running now.\r\n", CARD_SEQ_MAX);
        s_state = CM_READY_TO_RUN;
        return;
    }

    uart_printf("[CARD] Push %s (len=%u).\r\n", op_to_str(op), s_len);

    // 꽉 찼다면 자동으로 READY 전환
    if (s_len >= CARD_SEQ_MAX)
    {
        s_state = CM_READY_TO_RUN;
        uart_printf("[CARD] Reached max %u. Trigger to RUN.\r\n", CARD_SEQ_MAX);
    }
}

bool card_mode_get_next_plan(color_t *plan_color_out)
{
    if (!s_active || (s_state != CM_RUNNING))
    {
        return false;
    }

    // ★ 인터스텝 지연 체크: 대기 중이면 아직 plan을 안 준다
	if (s_gap_waiting)
	{
		uint32_t now = millis();
		// uint32_t 오버플로우는 자연 감산 비교로도 보정 가능하지만
		// 여기선 단순 비교로 충분(1초 수준)
		if ((int32_t)(now - s_gap_deadline) < 0)
		{
			return false;  // 아직 대기 중
		}

		// 대기 완료
		s_gap_waiting = false;
	}

    if (s_plan_ready)
    {
        // 아직 ap가 plan_started 콜백으로 소비 안 했으면 그대로 둠
        if (plan_color_out != NULL)
        {
            *plan_color_out = s_plan_color;
        }
        return true;
    }

    // 실행할 다음 항목이 남아 있으면 꺼내고 실행용 색으로 변환
    if (s_exec_idx < s_len)
    {
        card_op_t op = s_seq[s_exec_idx];

        if (op_to_plan_color(op, &s_plan_color))
        {
            s_plan_ready = true;
            if (plan_color_out != NULL)
            {
                *plan_color_out = s_plan_color;
            }
            uart_printf("[CARD] Plan #%u/%u -> %s\r\n",
                        (unsigned)(s_exec_idx + 1),
                        (unsigned)s_len,
                        color_to_string(s_plan_color));
            return true;
        }
        else
        {
            // 매핑 실패는 스킵하고 다음으로
            ++s_exec_idx;
            return card_mode_get_next_plan(plan_color_out);
        }
    }

    // 모두 소비 완료 → 종료
    uart_printf("[CARD] RUN finished.\r\n");
    card_mode_reset();
    return false;
}

void card_mode_on_motion_started(void)
{
    // ap가 plan을 태웠으면 소비 완료로 보고 인덱스 진행
    if (s_active && (s_state == CM_RUNNING) && s_plan_ready)
    {
        s_plan_ready = false;
        ++s_exec_idx;
    }
}

void card_mode_on_motion_finished(void)
{
    // 기존: RUN 끝나면 reset만 하거나, 다음으로 넘어가게 했었음
    // 여기서 "다음 plan을 내주기 전에 s_gap_ms 만큼 기다리도록" 플래그만 세운다.

    // 실행 중이 아니면 무시
    if (!(s_active && (s_state == CM_RUNNING)))
    {
        return;
    }

    // 아직 실행 버퍼가 남아 있는지 확인
    // 주의: s_exec_idx는 card_mode_on_motion_started()에서 이미 +1 되었음
    if (s_exec_idx < s_len)
    {
        // 다음 plan 앞서 "대기"
        s_gap_waiting  = true;
        s_gap_deadline = millis() + (uint32_t)s_gap_ms;
        uart_printf("[CARD] Gap %ums before next step.\r\n", (unsigned)s_gap_ms);
        return;
    }

    // 모두 소비 완료 → 종료
    uart_printf("[CARD] RUN finished.\r\n");
    card_mode_reset();
}


bool card_mode_is_programming(void)
{
    return s_active && (s_state == CM_PROGRAMMING || s_state == CM_READY_TO_RUN);
}

bool card_mode_is_running(void)
{
    return s_active && (s_state == CM_RUNNING);
}

void card_mode_set_interstep_delay_ms(uint16_t ms)
{
    s_gap_ms = ms;
}

uint16_t card_mode_get_interstep_delay_ms(void)
{
    return s_gap_ms;
}

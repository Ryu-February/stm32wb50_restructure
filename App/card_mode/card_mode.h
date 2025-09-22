/*
 * card_mode.h
 *
 *  Created on: Sep 18, 2025
 *      Author: RCY
 */

#ifndef CARD_MODE_CARD_MODE_H_
#define CARD_MODE_CARD_MODE_H_


#include "def.h"
#include "color.h"


#define CARD_SEQ_MAX   50

typedef enum
{
    CARD_OP_NONE = 0,
    CARD_OP_FORWARD,
    CARD_OP_REVERSE,
    CARD_OP_TURN_RIGHT,
    CARD_OP_TURN_LEFT
} card_op_t;

/* 초기화/리셋 */
void card_mode_init(void);
void card_mode_reset(void);

/* 모드 상태 */
bool card_mode_is_active(void);
uint16_t card_mode_seq_size(void);   // 현재 버퍼 길이
void card_mode_seq_clear(void);

/* 매핑 설정 (입력카드 -> 동작)
   기본값: RED->FWD, GREEN->REV, YELLOW->RIGHT, BLUE->LEFT */
void card_mode_set_input_mapping(color_t color, card_op_t op);

/* 색상 이벤트 피드 */
void card_mode_on_dual_colors(uint8_t left, uint8_t right);   // 트리거(LEFT=GREEN, RIGHT=ORANGE) 감지
void card_mode_on_detected_color(color_t color);               // 동일색 확정 시 호출(시퀀스 입력)

/* 실행 훅: ap_motion_update()에서 호출
   실행할 계획이 있으면 true와 함께 plan_color_out에 넣어줌
   plan_color_out는 motion_plan_color()에 그대로 전달 가능한 컬러(실행용 매핑) */
bool card_mode_get_next_plan(color_t *plan_color_out);

/* ap가 계획을 태우고/끝냈을 때 알려주기 */
void card_mode_on_motion_started(void);
void card_mode_on_motion_finished(void);

bool card_mode_is_programming(void);  // CM_PROGRAMMING ?
bool card_mode_is_running(void);      // CM_RUNNING ?





#endif /* CARD_MODE_CARD_MODE_H_ */

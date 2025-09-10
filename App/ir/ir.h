/*
 * ir.h
 *
 *  Created on: Sep 8, 2025
 *      Author: RCY
 */

#ifndef IR_IR_H_
#define IR_IR_H_



#include "def.h"

#define IR_THRESHOLD 	30



uint16_t ir_read_adc(void);
uint8_t ir_is_black(void);



#endif /* IR_IR_H_ */

/*
 * ap_isr.h
 *
 *  Created on: Jul 28, 2025
 *      Author: fbcks
 */

#ifndef AP_ISR_H_
#define AP_ISR_H_


#include "def.h"




void ap_exti0_callback(void);
void ap_tim2_callback(void);
void ap_tim16_callback(void);
void ap_tim17_callback(void);


#endif /* AP_ISR_H_ */

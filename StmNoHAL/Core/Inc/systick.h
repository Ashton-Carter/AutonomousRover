/*
 * systick.h
 *
 *  Created on: Jan 26, 2026
 *      Author: ashtoncarter
 */

#ifndef SRC_SYSTICK_H_
#define SRC_SYSTICK_H_
#include <stdint.h>
void SysTick_Handler(void);

uint32_t get_ms(void);

#endif /* SRC_SYSTICK_H_ */

/*
 * systick.c
 *
 *  Created on: Jan 26, 2026
 *      Author: ashtoncarter
 */

#include "systick.h"
volatile uint32_t msTick = 0;


void SysTick_Handler(void) {
	msTick++;
}

uint32_t get_ms(void){
	return msTick;
}

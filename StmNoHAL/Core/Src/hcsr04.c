#include "hcsr04.h"

#include <pin_config.h>
#include "globals.h"

typedef struct {
	volatile uint8_t waiting_for_fall;
	volatile uint8_t measurement_ready;
	volatile uint32_t rising_edge_us;
	volatile uint32_t pulse_width_us;
} hcsr04_state_t;

static hcsr04_state_t hcsr04_state = {0};

static void hcsr04_delay_us(uint32_t delay_us){
	uint32_t start = TIM5->CNT;

	while((uint32_t)(TIM5->CNT - start) < delay_us){
		__asm__("nop");
	}
}

void hcsr04_init(void){
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
	TIM5->CR1 = 0;
	TIM5->PSC = HC_SR04_TIMER_PRESCALER;
	TIM5->ARR = 0xFFFFFFFFUL;
	TIM5->EGR = TIM_EGR_UG;
	TIM5->CR1 |= TIM_CR1_CEN;

	pin_init(HC_SR04_TRIGGER_PORT, HC_SR04_TRIGGER_PIN, GPIO_OUTPUT, 0);
	set_gpio_pin(HC_SR04_TRIGGER_PORT, HC_SR04_TRIGGER_PIN, 0);
	pin_init_input(HC_SR04_ECHO_PORT, HC_SR04_ECHO_PIN, GPIO_PULL_DOWN);

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI7;
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PC;

	EXTI->IMR1 |= EXTI_IMR1_IM7;
	EXTI->RTSR1 |= EXTI_RTSR1_RT7;
	EXTI->FTSR1 |= EXTI_FTSR1_FT7;
	EXTI->PR1 = EXTI_PR1_PIF7;

	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

int hcsr04_start_measurement(void){
	if(hcsr04_state.waiting_for_fall){
		return -1;
	}

	hcsr04_state.measurement_ready = 0;
	hcsr04_state.pulse_width_us = 0;

	set_gpio_pin(HC_SR04_TRIGGER_PORT, HC_SR04_TRIGGER_PIN, 1);
	hcsr04_delay_us(12);
	set_gpio_pin(HC_SR04_TRIGGER_PORT, HC_SR04_TRIGGER_PIN, 0);
	return 0;
}

void hcsr04_handle_exti_irq(void){
	if(!(EXTI->PR1 & EXTI_PR1_PIF7)){
		return;
	}

	EXTI->PR1 = EXTI_PR1_PIF7;

	if(HC_SR04_ECHO_PORT->IDR & (1U << HC_SR04_ECHO_PIN)){
		hcsr04_state.waiting_for_fall = 1;
		hcsr04_state.rising_edge_us = TIM5->CNT;
		return;
	}

	if(hcsr04_state.waiting_for_fall){
		hcsr04_state.pulse_width_us = (uint32_t)(TIM5->CNT - hcsr04_state.rising_edge_us);
		hcsr04_state.waiting_for_fall = 0;
		hcsr04_state.measurement_ready = 1;
	}
}

uint8_t hcsr04_measurement_ready(void){
	return hcsr04_state.measurement_ready;
}

uint32_t hcsr04_get_pulse_width_us(void){
	return hcsr04_state.pulse_width_us;
}

uint32_t hcsr04_get_distance_mm(void){
	return (hcsr04_state.pulse_width_us * 343U) / 2000U;
}

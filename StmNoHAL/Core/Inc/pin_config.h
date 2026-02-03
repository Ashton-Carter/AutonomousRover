/*
 * gpio.h
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */

#ifndef INC_PIN_CONFIG_H_
#define INC_PIN_CONFIG_H_

#include "stm32g4xx.h"
#include <stdint.h>

#define GPIO_OUTPUT 1U
#define GPIO_ALTERNATIVE 2U

void pin_init(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t alternative_function);

void set_gpio_pin(GPIO_TypeDef *port, uint8_t pin, uint8_t val);

void enable_timer(uint8_t timer, uint8_t prescaler, uint16_t arr);

void set_pwm(TIM_TypeDef *timer, uint16_t period);


#endif /* INC_PIN_CONFIG_H_ */

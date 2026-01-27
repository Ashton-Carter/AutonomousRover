/*
 * gpio.h
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */

#ifndef INC_GPIO_H_
#define INC_GPIO_H_

#include "stm32g4xx.h"
#include <stdint.h>

#define GPIO_OUTPUT 1U
#define GPIO_ALTERNATIVE 2U

void gpio_init(GPIO_TypeDef *port, uint8_t pin, uint8_t mode);

void set_gpio_pin(GPIO_TypeDef *port, uint8_t pin, uint8_t val);


#endif /* INC_GPIO_H_ */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "stm32g4xx.h"
#include <stdint.h>

void i2c1_init(void);
int i2c_write_bytes(I2C_TypeDef *i2c, uint8_t address7, const uint8_t *data, uint8_t length);
int i2c_write_register(I2C_TypeDef *i2c, uint8_t address7, uint8_t reg, uint8_t value);
int i2c_read_registers(I2C_TypeDef *i2c, uint8_t address7, uint8_t reg, uint8_t *buffer, uint8_t length);

#endif /* INC_I2C_H_ */

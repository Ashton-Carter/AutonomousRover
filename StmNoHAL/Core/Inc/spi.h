/*
 * spi.h
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "stm32g4xx.h"


void spi_init(SPI_TypeDef *spi);
int spi_trasmit_recieve(uint8_t transmitBuffer[], uint8_t recieveBuffer[], uint8_t msgLen);

#endif /* INC_SPI_H_ */

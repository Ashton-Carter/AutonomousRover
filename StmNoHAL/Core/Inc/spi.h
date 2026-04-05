/*
 * spi.h
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "stm32g4xx.h"

#define MESSAGE_LEN 4
extern uint8_t RX_BUFFER[MESSAGE_LEN];
extern uint8_t TX_BUFFER[MESSAGE_LEN];

extern volatile uint8_t buffer_idx;
extern volatile uint8_t dirty;


void spi_init(SPI_TypeDef *spi);
void set_tx_buffer(uint16_t horizontalPWM, uint16_t verticalPWM);
int spi_trasmit_recieve(uint8_t transmitBuffer[], uint8_t recieveBuffer[], uint8_t msgLen);

#endif /* INC_SPI_H_ */

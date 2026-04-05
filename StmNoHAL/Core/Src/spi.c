/*
 * spi.c
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */
#include <pin_config.h>
#include "spi.h"

uint8_t RX_BUFFER[MESSAGE_LEN] = {};
uint8_t TX_BUFFER[MESSAGE_LEN] = {0xF1, 0xF2, 0xF3, 0xF4};

volatile uint8_t buffer_idx = 0;
volatile uint8_t dirty = 0;

void set_tx_buffer(uint16_t horizontalPWM, uint16_t verticalPWM){
	for(int i = 0; i < (MESSAGE_LEN/2); ++i){
		TX_BUFFER[i] = horizontalPWM & (0xFF << (i * 8));
		TX_BUFFER[i+MESSAGE_LEN/2] = verticalPWM & (0xFF << (i * 8));
	}
}

void spi_init(SPI_TypeDef *spi){
	// Enable clock for spi1
	pin_init(GPIOA, 4, GPIO_ALTERNATIVE, 5);
	pin_init(GPIOA, 5, GPIO_ALTERNATIVE, 5);
	pin_init(GPIOA, 6, GPIO_ALTERNATIVE, 5);
	pin_init(GPIOA, 7, GPIO_ALTERNATIVE, 5);

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	// Disable SPI for config
	spi->CR1 &= ~SPI_CR1_SPE;


	// Set clock polarity to low
	spi->CR1 &= ~SPI_CR1_CPOL;

	// Set clock phase to one
	spi->CR1 &= ~SPI_CR1_CPHA;

	// Use 2 wires for communication
	spi->CR1 &= ~SPI_CR1_BIDIMODE;

	// Set to MSB first
	spi->CR1 &= ~SPI_CR1_LSBFIRST;

	// Clear SSM and clear SSI
	spi->CR1 &= ~SPI_CR1_SSM;
	spi->CR1 &= ~SPI_CR1_SSI;

	// Set as slave, PI will be master
	spi->CR1 &= ~SPI_CR1_MSTR;


	// Clear data length
	spi->CR2 &= ~SPI_CR2_DS;
	// Set data length to 8 bits
	spi->CR2 |= (7U << SPI_CR2_DS_Pos);

	// Set FIFO threshold for interupt to 8 bits
	spi->CR2 |= SPI_CR2_FRXTH;

	// Set to Motorola frame format
	spi->CR2 &= ~SPI_CR2_FRF;


	// Enable recieve interrupt
	spi->CR2 |= SPI_CR2_RXNEIE;

	// Enable SPI
	spi->CR1 |= SPI_CR1_SPE;
	NVIC_EnableIRQ(SPI1_IRQn);


}

void SPI1_IRQHandler(void){
	uint32_t sr = SPI1->SR;

	if(sr & SPI_SR_RXNE){
		RX_BUFFER[buffer_idx] = SPI1->DR;
		buffer_idx = (buffer_idx + 1) % 4;

		SPI1->DR = TX_BUFFER[buffer_idx];

		if (!buffer_idx) {
			dirty = 1;
		}
	}
}

/*
 * spi.c
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */
#include <pin_config.h>
#include "spi.h"
#include "globals.h"

uint8_t RX_BUFFER[MESSAGE_LEN] = {};
uint8_t TX_BUFFER[MESSAGE_LEN] = {0x11, 0x22, 0x33, 0x44};
uint8_t transmitInterupts = 0;

volatile uint8_t buffer_idx = 0;
static volatile uint32_t tx_idx = 0;
volatile uint8_t dirty = 0;

static inline void spi_write8(SPI_TypeDef *spi, uint8_t value){
	*(__IO uint8_t *)&spi->DR = value;
}


static inline uint8_t spi_read8(SPI_TypeDef *spi){
	return *(__IO uint8_t *)&spi->DR;
}


void set_tx_buffer(uint16_t horizontalPWM, uint16_t verticalPWM, uint8_t distance){

	spi_write8(SPI1, (uint8_t) ((horizontalPWM & 0xFF0) >> 4));
	spi_write8(SPI1, (uint8_t) (((horizontalPWM & 0x00F) << 4) | ((verticalPWM & 0xF00) >> 8)));
	spi_write8(SPI1, (uint8_t) (verticalPWM & 0xFF));
	spi_write8(SPI1, (uint8_t) (distance));

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
//	spi->CR2 |= SPI_CR2_TXEIE;


	// Enable SPI
	spi->CR1 |= SPI_CR1_SPE;

	NVIC_EnableIRQ(SPI1_IRQn);
	buffer_idx = 0;
}

void SPI1_IRQHandler(void){
	uint32_t sr = SPI1->SR;

//	if(sr & SPI_SR_TXE){
//		transmitInterupts++;
//		spi_write8(SPI1, TX_BUFFER[tx_idx]);
//		tx_idx = (tx_idx + 1) % MESSAGE_LEN;
//	}

	if(sr & SPI_SR_UDR){
		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
	}

	if(sr & SPI_SR_OVR){
		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
	}

	if(sr & SPI_SR_RXNE){
		RX_BUFFER[buffer_idx] = spi_read8(SPI1);
		buffer_idx = (buffer_idx + 1) % MESSAGE_LEN;

		if (!buffer_idx) {
			dirty = 1;
		}
	}
}



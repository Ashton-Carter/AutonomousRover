/*
 * spi.c
 *
 *  Created on: Jan 25, 2026
 *      Author: ashtoncarter
 */
#include "spi.h"
#include "gpio.h"

#define MESSAGE_LEN 4

uint8_t RX_BUFFER[MESSAGE_LEN];
uint8_t TX_BUFFER[MESSAGE_LEN] = {0};

volatile uint8_t buffer_idx = 0;
volatile uint8_t dirty = 0;

void spi_init(SPI_TypeDef *spi){
	// Enable clock for spi1
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	// Disable SPI for config
	spi->CR1 &= ~SPI_CR1_SPE;

	// Set as slave, PI will be master
	spi->CR1 &= ~SPI_CR1_MSTR;

	// Set clock polarity to low
	spi->CR1 &= ~SPI_CR1_CPOL;

	// Set clock phase to one
	spi->CR1 &= ~SPI_CR1_CPHA;

	// Clear data length
	spi->CR2 &= ~SPI_CR2_DS;

	// Set data length to 8 bits
	spi->CR2 |= (7U << SPI_CR2_DS_Pos);

	// Set to MSB first
	spi->CR1 &= ~SPI_CR1_LSBFIRST;

	// Set to Motorola frame format
	spi->CR2 &= ~SPI_CR2_FRF;

	// Set SSM and clear SSI
	spi->CR1 |= SPI_CR1_SSM;
	spi->CR1 &= ~SPI_CR1_SSI;

	// Enable recieve interrupt
	spi->CR2 |= SPI_CR2_RXNEIE ;
	NVIC_EnableIRQ(SPI1_IRQn);

	// Enable SPI
	spi->CR1 |= SPI_CR1_SPE;

	gpio_init(GPIOA, 5, GPIO_ALTERNATIVE);
	gpio_init(GPIOA, 6, GPIO_ALTERNATIVE);
	gpio_init(GPIOA, 7, GPIO_ALTERNATIVE);
}

void SPI1_IRQHandler(void){

	uint32_t sr = SPI1->SR;

	if(sr & SPI_SR_RXNE){
			RX_BUFFER[buffer_idx] = SPI1->DR;
		}

	if(sr & SPI_SR_TXE){
		SPI1->DR = TX_BUFFER[buffer_idx];
	}

	buffer_idx = (buffer_idx + 1) % 4;
	if (!buffer_idx) {
		dirty = 1;
	}

}

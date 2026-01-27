#include "gpio.h"


void gpio_init(GPIO_TypeDef *port, uint8_t pin, uint8_t mode){

	// Enable clock on AHB2 bus ENR = enable register
	uint8_t offset;
	if(port == GPIOA){
		offset = 0;
	} else if (port == GPIOB){
		offset = 1;
	} else if(port == GPIOC){
		offset = 2;
	} else if(port == GPIOD) {
		offset = 3;
	} else if (port == GPIOE) {
		offset = 4;
	} else if (port == GPIOF) {
		offset = 5;
	} else if (port == GPIOG) {
		offset = 6;
	}
	RCC->AHB2ENR = RCC->AHB2ENR | (0x1UL << offset);


	//First clear any potential bits from the mode by creating a mask on the bits for port
	port->MODER = port->MODER & ~(3U << (2*pin));
	// Set pin equal to 01, which correlates to output
	port->MODER = port->MODER | (mode  << (2*pin));

	// Set output type to push-pull on the pin, one bit per pin, again we can create a mask
	port->OTYPER = port->OTYPER & ~(1U << pin);

	// Set the speed of the register, currently an LED so no need for speed.
	port->OSPEEDR = port->OSPEEDR & ~(3U << (2*pin));

	//Sets pull up/down behavior
	port->PUPDR = port->PUPDR & ~(3U << (2*pin));
	if(mode == GPIO_OUTPUT){
		//Sets initial behavior of the pin
		port->ODR = port->ODR & ~(1U << pin);
	} else if (mode==GPIO_ALTERNATIVE) {
		if(pin < 8){
			port->AFR[0] &= ~(0xF << 4*pin);
			port->AFR[0] |= 5U << 4*pin;
		} else {
			port->AFR[1] &= ~(0xF << 4*(pin-8));
			port->AFR[1] |= 5U << 4*(pin-8);
		}

	}


}

void set_gpio_pin(GPIO_TypeDef *port, uint8_t pin, uint8_t val){
	//if value is 0, set pin to LOW and return


	if(!val){
		//BSRR, upper 16 bits for low, first 16 for high
		port->BSRR = 1U << (pin+16);
		return;
	}

	//value is 0
	port->BSRR = 1U << pin;


}

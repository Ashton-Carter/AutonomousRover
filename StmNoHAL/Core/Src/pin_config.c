#include "pin_config.h"


void pin_init(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t alternative_function){

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
	RCC->AHB2ENR = RCC->AHB2ENR | (1UL << offset);


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
			port->AFR[0] |= alternative_function << 4*pin;
		} else {
			port->AFR[1] &= ~(0xF << 4*(pin-8));
			port->AFR[1] |= alternative_function << 4*(pin-8);
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

void enable_timer(uint8_t timer, uint8_t prescaler, uint16_t arr){
	if(timer < 2){
		return;
	}
	RCC->APB1ENR1 |= 1U << (timer-2);
	TIM_TypeDef *timr = (TIM_TypeDef *)(APB1PERIPH_BASE + (0x0400UL * (timer-2)));

	timr->CR1 = 0;
	timr->PSC = prescaler;
	timr->ARR = arr;
	timr->CCMR1 &= ~TIM_CCMR1_CC1S;
	timr->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos);
	timr->CCMR1 |= TIM_CCMR1_OC1PE;
	timr->CR1 |= TIM_CR1_ARPE;
	timr->CCR1 = 1500;
	timr->EGR |= TIM_EGR_UG;
	timr->CCER |= TIM_CCER_CC1E;
	timr->CR1 |= TIM_CR1_CEN;

}

void set_pwm(TIM_TypeDef *timer, uint16_t period){
		timer->CCR1 = period;
}


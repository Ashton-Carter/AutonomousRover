#include "stm32g4xx.h"
#include "gpio.h"
#include "spi.h"

static void busy_wait(int ms);


int main(void)
{
//    gpio_init(GPIOC, 10);
    busy_wait(10000);
//    set_gpio_pin(GPIOC, 10, 1);
//    busy_wait(10000);
//    set_gpio_pin(GPIOC, 10, 0);

    spi_init(SPI1);


    while (1)
    {
        // main loop
    }
}


static void busy_wait(int ms){
	// Very approximate, do not use for necessary delays
	volatile int cycles = ms/0.0005;
	while(--cycles){
		__asm__("nop");
	}
}

#include "stm32g4xx.h"
#include "gpio.h"
#include "spi.h"
#include "systick.h"

#define FORWARD 0x10
#define LEFT 0x11
#define RIGHT 0x12
#define BACK 0x13
#define CLOSE 0xFF
#define BLUE_LED 10
#define RED_LED 11
#define GREEN_LED 12

static void busy_wait(int ms);
void handleMovement(uint8_t Direction);
void translateDurationAmount(uint8_t RX_BUFFER[MESSAGE_LEN]);

uint32_t instruction_timers[4] = {0};

int main(void)
{
	// Set clock interupt to fire every millisecond
	SysTick_Config(SystemCoreClock/1000);

    gpio_init(GPIOC, BLUE_LED, GPIO_OUTPUT);
    gpio_init(GPIOC, RED_LED, GPIO_OUTPUT);
    gpio_init(GPIOC, GREEN_LED, GPIO_OUTPUT);
    busy_wait(10);
//    set_gpio_pin(GPIOC, BLUE_LED, 1);
//    set_gpio_pin(GPIOC, 10, 1);
//    busy_wait(10000);
//    set_gpio_pin(GPIOC, 10, 0);

    spi_init(SPI1);

    uint32_t currentTime;
    while (1)
    {
        if(dirty){
        	translateDurationAmount(RX_BUFFER);
        	dirty = 0;
        }

        currentTime = get_ms();
        for(int i = 0; i < 4; ++i){
        	if(instruction_timers[i] > currentTime){
        		set_gpio_pin(GPIOC, i + 1, 1);
        	} else {
        		set_gpio_pin(GPIOC, i + 1, 0);
        	}
        }
    }
}




static void busy_wait(int ms){
	// Very approximate, do not use for necessary delays
	volatile int cycles = ms/0.0005;
	while(--cycles){
		__asm__("nop");
	}
}


void translateDurationAmount(uint8_t RX_BUFFER[MESSAGE_LEN]){
	instruction_timers[RX_BUFFER[0]-0x10] = get_ms() + ((RX_BUFFER[1]<<16) | (RX_BUFFER[2]<<8) | RX_BUFFER[3]);
}

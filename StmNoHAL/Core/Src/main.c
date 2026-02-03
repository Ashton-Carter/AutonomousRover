#include <pin_config.h>
#include "stm32g4xx.h"
#include "spi.h"
#include "systick.h"

#define CAMERA_UP 0x01
#define CAMERA_DOWN 0x02
#define CAMERA_LEFT 0x03
#define CAMERA_RIGHT 0x04
#define FORWARD 0x10
#define LEFT 0x11
#define RIGHT 0x12
#define BACK 0x13
#define CLOSE 0xFF
#define BLUE_LED 10
#define RED_LED 11
#define GREEN_LED 12
#define PRESCALER 15
#define ARR 19999

#define PWN_OUTPUT 0x00

static void wait(int ms);
void handleMovement(uint8_t Direction);
void translateDurationAmount(uint8_t RX_BUFFER[MESSAGE_LEN]);

uint32_t instruction_timers[4] = {0};

int main(void)
{
	// Set clock interupt to fire every millisecond
	SysTick_Config(SystemCoreClock/1000);

	// 0 inconsequential with gpio_output
    pin_init(GPIOC, BLUE_LED, GPIO_OUTPUT, 0);
    pin_init(GPIOC, RED_LED, GPIO_OUTPUT, 0);
    pin_init(GPIOC, GREEN_LED, GPIO_OUTPUT, 0);

    spi_init(SPI1);

    pin_init(GPIOA, PWN_OUTPUT, GPIO_ALTERNATIVE, 1);
    enable_timer(2, PRESCALER, ARR);
    wait(500);


//    set_gpio_pin(GPIOC, 10, 1);
//    busy_wait(10000);
//    set_gpio_pin(GPIOC, 10, 0);


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




static void wait(int ms){
	uint32_t end = get_ms() + ms;
	while(get_ms() < end){
		__asm__("nop");
	}
}


void translateDurationAmount(uint8_t RX_BUFFER[MESSAGE_LEN]){
	uint32_t amount = ((RX_BUFFER[1]<<16) | (RX_BUFFER[2]<<8) | RX_BUFFER[3]);
	if (RX_BUFFER[0] >= 0x10){
		instruction_timers[RX_BUFFER[0]-0x10] = get_ms() + amount;
		return;
	}
	switch(RX_BUFFER[0]){
	case(CAMERA_LEFT):
		set_pwm(TIM2, 1500-amount);
		break;
	case(CAMERA_RIGHT):
		set_pwm(TIM2, 1500+amount);
		break;
	}
}

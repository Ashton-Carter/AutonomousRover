#include <pin_config.h>
#include "stm32g4xx.h"
#include "spi.h"
#include "systick.h"
#include "globals.h"


static void wait(int ms);
void handleMovement(uint8_t Direction);
void translateDurationAmount(uint8_t RX_BUFFER[MESSAGE_LEN]);

uint32_t instruction_timers[4] = {0};

int main(void)
{
	// Set clock interupt to fire every millisecond
	SysTick_Config(SystemCoreClock/1000);

	// 0 inconsequential with gpio_output
    pin_init(GPIOC, RIGHT_BACKWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, RIGHT_FORWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, LEFT_FORWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, LEFT_BACKWARD, GPIO_OUTPUT, 0);

    spi_init(SPI1);

    pin_init(GPIOA, SERVO_PWM_OUTPUT, GPIO_ALTERNATIVE, 1);
    enable_timer(2, SERVO_PRESCALER, SERVO_ARR);
    set_pwm(TIM2, 1500);

    pin_init(GPIOA, MOTOR_PWM_OUTPUT, GPIO_ALTERNATIVE, 2);
    enable_timer(3, MOTOR_PRESCALER, MOTOR_ARR);
    set_pwm(TIM3, 700);
    set_gpio_pin(GPIOC, 10, 0);
    set_gpio_pin(GPIOC, 11, 0);
    set_gpio_pin(GPIOC, 12, 0);
    set_gpio_pin(GPIOC, 13, 0);

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
        		set_gpio_pin(GPIOC, i + 10, 1);
        	} else {
        		set_gpio_pin(GPIOC, i + 10, 0);
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
	uint32_t current = get_ms();
	uint32_t futureTime = current + amount;

	switch(RX_BUFFER[0]){
	case(FORWARD):
		instruction_timers[LEFT_FORWARD_POSITION] = futureTime;
		instruction_timers[LEFT_BACKWARD_POSITION] = current;
		instruction_timers[RIGHT_FORWARD_POSITION] = futureTime;
		instruction_timers[RIGHT_BACKWARD_POSITION] = current;
		break;
	case(BACK):
		instruction_timers[RIGHT_FORWARD_POSITION] = current;
		instruction_timers[RIGHT_BACKWARD_POSITION] = futureTime;
		instruction_timers[LEFT_FORWARD_POSITION] = current;
		instruction_timers[LEFT_BACKWARD_POSITION] = futureTime;
		break;
	case(LEFT):
		instruction_timers[RIGHT_FORWARD_POSITION] = futureTime;
		instruction_timers[RIGHT_BACKWARD_POSITION] = current;
		instruction_timers[LEFT_FORWARD_POSITION] = current;
		instruction_timers[LEFT_BACKWARD_POSITION] = futureTime;
		break;
	case(RIGHT):
		instruction_timers[RIGHT_FORWARD_POSITION] = current;
		instruction_timers[RIGHT_BACKWARD_POSITION] = futureTime;
		instruction_timers[LEFT_FORWARD_POSITION] = futureTime;
		instruction_timers[LEFT_BACKWARD_POSITION] = current;
		break;
	case(CAMERA_LEFT):
		set_pwm(TIM2, 1500-amount);
		break;
	case(CAMERA_RIGHT):
		set_pwm(TIM2, 1500+amount);
		break;
	}
}

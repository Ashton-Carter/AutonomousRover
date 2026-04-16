#include <pin_config.h>
#include "stm32g4xx.h"
#include "spi.h"
#include "systick.h"
#include "globals.h"
#include "hcsr04.h"

static void wait(int ms);
void translateInstructionsAmount(uint8_t RX_BUFFER[MESSAGE_LEN]);

uint32_t instruction_timers[INSTRUCTION_TIMERS];
uint32_t index_to_gpio_pin[INSTRUCTION_TIMERS];

uint16_t verticalPeriod = VERTICAL_MIN_SERVO;
uint16_t horizontalPeriod = MIDPOINT_SERVO;
uint32_t distance = 0;
static uint32_t next_hcsr04_measurement_ms = 0;

int main(void)
{
	// Set clock interupt to fire every millisecond
	SysTick_Config(SystemCoreClock/1000);

	for (int i = 0; i < INSTRUCTION_TIMERS; ++i){
		instruction_timers[i] = 0;
	}

	index_to_gpio_pin[RIGHT_FORWARD_POSITION] = RIGHT_FORWARD;
	index_to_gpio_pin[RIGHT_BACKWARD_POSITION] = RIGHT_BACKWARD;
	index_to_gpio_pin[LEFT_FORWARD_POSITION] = LEFT_FORWARD;
	index_to_gpio_pin[LEFT_BACKWARD_POSITION] = LEFT_BACKWARD;
	index_to_gpio_pin[FIRE_POSITION] = LASER;

	// 0 inconsequential with gpio_output
    pin_init(GPIOC, RIGHT_BACKWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, RIGHT_FORWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, LEFT_FORWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, LEFT_BACKWARD, GPIO_OUTPUT, 0);
    pin_init(GPIOC, LASER, GPIO_OUTPUT, 0);

    spi_init(SPI1);

    pin_init(GPIOA, SERVO_PWM_OUTPUT, GPIO_ALTERNATIVE, 1);
    enable_timer(2, SERVO_PRESCALER, SERVO_ARR);
    set_pwm(VERTICAL_SERVO, verticalPeriod);

    pin_init(GPIOB, MOTOR_PWM_OUTPUT, GPIO_ALTERNATIVE, 2);
    enable_timer(3, MOTOR_PRESCALER, MOTOR_ARR);
    set_pwm(TIM3, 600);
    set_gpio_pin(GPIOC, RIGHT_BACKWARD, 0);
    set_gpio_pin(GPIOC, RIGHT_FORWARD, 0);
    set_gpio_pin(GPIOC, LEFT_FORWARD, 0);
    set_gpio_pin(GPIOC, LEFT_BACKWARD, 0);
    wait(500);

    pin_init(GPIOB, SERVO2_PWM_OUTPUT, GPIO_ALTERNATIVE, 2);
    enable_timer(4, SERVO_PRESCALER, SERVO_ARR);
    set_pwm(HORIZONTAL_SERVO, horizontalPeriod);
    set_gpio_pin(GPIOC, LASER, 0);
    hcsr04_init();

//    set_gpio_pin(GPIOC, 10, 1);
//    busy_wait(10000);
//    set_gpio_pin(GPIOC, 10, 0);

    // Max and Min for servo is more like 500 - 2500


	    uint32_t currentTime;
	    while (1)
	    {
	        currentTime = get_ms();
	        if(currentTime >= next_hcsr04_measurement_ms){
	        	hcsr04_start_measurement();
	        	next_hcsr04_measurement_ms = currentTime + HC_SR04_MEASUREMENT_INTERVAL_MS;
	        }

	        if(dirty){
	        	translateInstructionsAmount(RX_BUFFER);
	        	dirty = 0;
	        }

	        for(int i = 0; i < INSTRUCTION_TIMERS; ++i){
	        	if(instruction_timers[i] > currentTime){
	        		set_gpio_pin(GPIOC, index_to_gpio_pin[i], 1);
        	} else {
        		set_gpio_pin(GPIOC, index_to_gpio_pin[i], 0);
        	}
	        }
	        if(hcsr04_measurement_ready()){
	        	distance = hcsr04_get_distance_inches();
	        }
	        set_tx_buffer(horizontalPeriod, verticalPeriod, distance);
	    }
}




static void wait(int ms){
	uint32_t end = get_ms() + ms;
	while(get_ms() < end){
		__asm__("nop");
	}
}


void translateInstructionsAmount(uint8_t RX_BUFFER[MESSAGE_LEN]){
	uint32_t amount = 0;
	for(int i = 1; i < MESSAGE_LEN; i++){
		amount |= RX_BUFFER[i] << ((MESSAGE_LEN-1-i) * 8);
	}

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
	case(FIRE):
		instruction_timers[FIRE_POSITION] = futureTime;
		break;
	case(CAMERA_RIGHT):
		horizontalPeriod -= amount;
		if(horizontalPeriod < HORIZONTAL_MIN_SERVO){
			horizontalPeriod = HORIZONTAL_MIN_SERVO;
		}

		set_pwm(HORIZONTAL_SERVO, horizontalPeriod);
		break;
	case(CAMERA_LEFT):
		horizontalPeriod += amount;
		if(horizontalPeriod > HORIZONTAL_MAX_SERVO){
			horizontalPeriod = HORIZONTAL_MAX_SERVO;
		}

		set_pwm(HORIZONTAL_SERVO, horizontalPeriod);
		break;
	case(CAMERA_DOWN):
		verticalPeriod -= amount;
		if(verticalPeriod < VERTICAL_MIN_SERVO){
			verticalPeriod = VERTICAL_MIN_SERVO;
		}

		set_pwm(VERTICAL_SERVO, verticalPeriod);
		break;
	case(CAMERA_UP):
		verticalPeriod += amount;
		if(verticalPeriod > VERTICAL_MAX_SERVO){
			verticalPeriod = VERTICAL_MAX_SERVO;
		}

		set_pwm(VERTICAL_SERVO, verticalPeriod);
		break;
	}
//	set_tx_buffer(0x1234, 0x5678, 0x9ABCDEF1);
//	set_tx_buffer(horizontalPeriod, verticalPeriod, distance);
}

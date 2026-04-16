#ifndef INC_HCSR04_H_
#define INC_HCSR04_H_

#include <stdint.h>

void hcsr04_init(void);
int hcsr04_start_measurement(void);
void hcsr04_handle_exti_irq(void);
uint8_t hcsr04_measurement_ready(void);
uint32_t hcsr04_get_pulse_width_us(void);
uint32_t hcsr04_get_distance_inches(void);

#endif /* INC_HCSR04_H_ */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include <stdint.h>

typedef struct {
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t temperature;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
} mpu6050_sample_t;

int mpu6050_init(void);
int mpu6050_read_who_am_i(uint8_t *who_am_i);
int mpu6050_read_sample(mpu6050_sample_t *sample);

#endif /* INC_MPU6050_H_ */

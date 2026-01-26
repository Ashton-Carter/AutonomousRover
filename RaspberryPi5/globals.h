#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>

#define MAX_IMAGE 100000
#define MAX_IMAGE_HEIGHT 480
#define MAX_IMAGE_WIDTH 640
#define TOTALSHAREDIMAGEMEMORY (sizeof(size_t) + MAX_IMAGE)
#define IMAGE_READ_AMOUNT 16000
#define SPI_BITS 8
#define SPI_LEN 4
#define SPI_SPEED 1000000
#define FORWARD 0x10
#define LEFT 0x11
#define RIGHT 0x12
#define BACK 0x13
#define CLOSE 0xFF
#define MANUAL_CONTROL_PORT 8000


#endif
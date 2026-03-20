#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>
#include <stdint.h>

#define SPI_BITS 8
#define SPI_LEN 4
#define SPI_SPEED 1000000
#define CAMERA_UP 0x01
#define CAMERA_DOWN 0x02
#define CAMERA_LEFT 0x03
#define CAMERA_RIGHT 0x04
#define FORWARD 0x05
#define LEFT 0x06
#define RIGHT 0x07
#define BACK 0x08
#define CLOSE 0x09
#define FIRE 0x0A
#define MANUAL_CONTROL_PORT 8000
#define UNIX_DOMAIN_SOCKET_PATH "/tmp/rover.sock"
#define SPI_BUFFER 5

struct threadStatus{
    uint8_t manualControl;
    uint8_t manualConnectionStatus;
    uint8_t pythonCVConnectionStatus;
};

extern struct threadStatus threadStatus;

#endif
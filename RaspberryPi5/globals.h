#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>
#include <stdint.h>

#define SPI_BITS 8
#define SPI_LEN 4
#define SPI_SPEED 1000000
#define FORWARD 0x10
#define LEFT 0x11
#define RIGHT 0x12
#define BACK 0x13
#define CLOSE 0xFF
#define CAMERA_UP 0x01
#define CAMERA_DOWN 0x02
#define CAMERA_LEFT 0x03
#define CAMERA_RIGHT 0x04
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
#ifndef SOCKET_LIFECYCLE_H
#define SOCKET_LIFECYCLE_H

#include <stdint.h>
#include "../../globals.h"

struct manualControlArgs{
    int port;
    uint8_t command;
    uint8_t amount[SPI_LEN-1];
    uint8_t changed;
    uint64_t last_update;
};

uint64_t now_ms();
void *socket_lifecycle(void *arg);


#endif // SOCKET_LIFECYCLE_H

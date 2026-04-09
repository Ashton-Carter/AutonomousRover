#ifndef SOCKET_LIFECYCLE_H
#define SOCKET_LIFECYCLE_H

#include <stdint.h>
#include "../../globals.h"

typedef struct{
    int port;
    uint8_t command;
    int amount;
    uint8_t changed;
}manualControlArgs;

uint64_t now_ms();
void *socket_lifecycle(void *arg);
int handleManualControl(manualControlArgs *args, uint8_t *command, unsigned int *timeOffset);


#endif // SOCKET_LIFECYCLE_H

#ifndef SPICONNECTION_H
#define SPICONNECTION_H
#include <pthread.h>
#include <stdint.h>
#include "../globals.h"

struct SPIArguments {
    int *dirty;
    pthread_mutex_t SPI_Buffer_Mutex;
    pthread_cond_t cond;
    uint8_t transmissionBuffer[SPI_LEN];
    targetingInformation* targetingInformation;
};

void *SPIHandler(void *arg);
int sendMessage(struct SPIArguments *arguments, uint8_t msg[SPI_LEN], int *dirty);



#endif
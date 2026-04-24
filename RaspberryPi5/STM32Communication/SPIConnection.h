#ifndef SPICONNECTION_H
#define SPICONNECTION_H
#include <pthread.h>
#include <stdint.h>
#include "../globals.h"

typedef struct {
    
    int trasmissionFreeIndex;
    int recieveFreeIndex;

    pthread_mutex_t transmissionMutex;
    pthread_mutex_t recieveMutex;

    uint8_t transmissionBuffer[SPI_BUFFER][SPI_LEN];
    uint8_t recieveBuffer[SPI_BUFFER][SPI_LEN];

} SPIArguments;

void *SPIHandler(void *arg);
int sendMessage(SPIArguments *arguments, uint8_t msg[SPI_LEN]);
int recieveMessage(SPIArguments *arguments, uint8_t recieveBuffer[SPI_LEN]);
void translateToBuffer(uint8_t buffer[SPI_LEN], uint8_t command, unsigned int time_offset);
void inputSpiMessages(uint8_t fromSpi[SPI_BUFFER][SPI_LEN], int messages, targetingInformation* targetingInformation);
// void sendResetMessage(int fd, struct spi_ioc_transfer* tr, uint8_t transferBuffer[SPI_LEN]);
// int checkSPI(uint8_t transferBuffer[SPI_LEN]);



#endif
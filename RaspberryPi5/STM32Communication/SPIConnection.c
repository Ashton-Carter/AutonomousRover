#include <linux/spi/spidev.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "../globals.h"
#include "SPIConnection.h"

void *SPIHandler(void *arg){
    SPIArguments *arguments = (SPIArguments *)arg;
    int fd = open("/dev/spidev0.0", O_RDWR);
    if(fd < 0){
        perror("ERROR OPENING SPI FILE\n");
        return NULL;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = SPI_BITS;
    uint32_t speed = SPI_SPEED;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    uint8_t transferBuffer[SPI_LEN] = {0};
    uint8_t receiveBuffer[SPI_LEN] = {0};
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&transferBuffer,
        .rx_buf = (unsigned long)&receiveBuffer,
        .len = SPI_LEN,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = 0,
    };
    
    while(1){
        pthread_mutex_lock(&arguments->transmissionMutex);
        pthread_mutex_lock(&arguments->recieveMutex);

        if(arguments->trasmissionFreeIndex){
            for(int i = 0; i < arguments->trasmissionFreeIndex; ++i){
                if(arguments->recieveFreeIndex >= SPI_BUFFER){
                    printf("TRANSFER BUFFER OVERFLOW\n");
                    break;
                }

                memcpy(transferBuffer, arguments->transmissionBuffer[i], SPI_LEN);
                if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                    perror("MESSAGE FAILURE\n");
                }
                printf("TRASMITTED:%X, %X, %X, %X\n", transferBuffer[0], transferBuffer[1], transferBuffer[2], transferBuffer[3]);
                if(arguments->recieveFreeIndex >= SPI_BUFFER){
                    printf("RECIEVE BUFFER OVERFLOW, OVERWRITING FROM INDEX 0\n");
                    arguments->recieveFreeIndex = 0;
                }
                memcpy(arguments->recieveBuffer[arguments->recieveFreeIndex], receiveBuffer, SPI_LEN);

                printf("RECIEVED:%X, %X, %X, %X\n", 
                    arguments->recieveBuffer[arguments->recieveFreeIndex][0], 
                    arguments->recieveBuffer[arguments->recieveFreeIndex][1], 
                    arguments->recieveBuffer[arguments->recieveFreeIndex][2], 
                    arguments->recieveBuffer[arguments->recieveFreeIndex][3]);
                arguments->recieveFreeIndex++;
            }
            arguments->trasmissionFreeIndex = 0;
        }
        pthread_mutex_unlock(&arguments->transmissionMutex);
        pthread_mutex_unlock(&arguments->recieveMutex);

    }
    close(fd);
    return 0;
}

int sendMessage(SPIArguments *arguments, uint8_t msg[SPI_LEN]){
    
    pthread_mutex_lock(&arguments->transmissionMutex);
    if(arguments->trasmissionFreeIndex >= SPI_BUFFER){
        pthread_mutex_unlock(&arguments->transmissionMutex);
        return -1;
    }
    memcpy(arguments->transmissionBuffer[arguments->trasmissionFreeIndex], msg, SPI_LEN);
    arguments->trasmissionFreeIndex++;
    pthread_mutex_unlock(&arguments->transmissionMutex);
    return SPI_BUFFER - arguments->trasmissionFreeIndex;
}

int recieveMessage(SPIArguments *arguments, uint8_t recieveBuffer[SPI_LEN]){
    
    pthread_mutex_lock(&arguments->recieveMutex);
    if(arguments->recieveFreeIndex <= 0){
        pthread_mutex_unlock(&arguments->recieveMutex);
        return -1;
    }
    memcpy(recieveBuffer, arguments->recieveBuffer[arguments->recieveFreeIndex-1], SPI_LEN);
    arguments->recieveFreeIndex--;
    pthread_mutex_unlock(&arguments->recieveMutex);
    return arguments->recieveFreeIndex;
}

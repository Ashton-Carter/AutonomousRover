#include <linux/spi/spidev.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "../globals.h"
#include "SPIConnection.h"


void translateToBuffer(uint8_t buffer[SPI_LEN], uint8_t command, unsigned int time_offset){
    buffer[0] = command;
    for(int i = SPI_LEN-1; i>=1; --i){
        buffer[SPI_LEN-i] = ((time_offset >> ((i-1)*8)) & 0xFF);
    }
}

void inputSpiMessages(uint8_t fromSpi[SPI_BUFFER][SPI_LEN], int messages, targetingInformation* targetingInformation){
    for(int i = 0; i < messages; ++i){
        uint32_t incomingMessage = 
        targetingInformation->last_horizontal_position = (fromSpi[i][0]<<4) | (((uint16_t)fromSpi[i][1] & 0xF0) >> 4);
        targetingInformation->last_vertical_position = (fromSpi[i][1] & 0x0F) << 8 | fromSpi[i][2];
        targetingInformation->distance = fromSpi[i][3];
    }
}

// void sendResetMessage(int fd, struct spi_ioc_transfer* tr, uint8_t transferBuffer[SPI_LEN]){
//     transferBuffer[0] = RESET;
//     if (ioctl(fd, SPI_IOC_MESSAGE(1), tr) < 1) {
//         perror("MESSAGE FAILURE ON RESET\n");
//     }
// }

// int checkSPI(uint8_t transferBuffer[SPI_LEN]){
//     unsigned int last_horizontal_position = (uint16_t)transferBuffer[0]<<8 | ((uint16_t)transferBuffer[1]);
//     unsigned int last_vertical_position = (uint16_t)transferBuffer[2]<<8 | ((uint16_t)transferBuffer[3]);
//     unsigned int distance = 
//     (uint32_t)(transferBuffer[4] << 24)|
//     (uint32_t)(transferBuffer[5] << 16)|
//     (uint32_t)(transferBuffer[6] << 8)|
//     (uint32_t)transferBuffer[7];

//     int impossibleValues = 
//     distance > DISTANCE_CAP ||
//     last_horizontal_position > SERVO_CAP ||
//     last_vertical_position > SERVO_CAP;
//     return impossibleValues;
// }

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
                // printf("TRANSMITTED:");
                // for(int i = 0; i < SPI_LEN; ++i){
                //     printf("%X, ", transferBuffer[i]);
                // }
                // printf("\n");
                if(arguments->recieveFreeIndex >= SPI_BUFFER){
                    printf("RECIEVE BUFFER OVERFLOW, OVERWRITING FROM INDEX 0\n");
                    arguments->recieveFreeIndex = 0;
                }
                memcpy(arguments->recieveBuffer[arguments->recieveFreeIndex], receiveBuffer, SPI_LEN);

                // printf("RECIEVED:");
                // for(int i = 0; i < SPI_LEN; ++i){
                //     printf("%X, ", arguments->recieveBuffer[arguments->recieveFreeIndex][i]);
                // }
                // printf("\n");
                // if(checkAndResetSPI(arguments->recieveBuffer[arguments->recieveFreeIndex])){
                //     sendResetMessage(fd, &tr, transferBuffer);
                // }
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


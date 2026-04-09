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
    struct SPIArguments *arguments = (struct SPIArguments *)arg;
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
        .len = 4,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = 0,
    };
    // int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    // if (ret < 1) {
    //     perror("SPI_IOC_MESSAGE");
    // }
    while(1){
        pthread_mutex_lock(&arguments->SPI_Buffer_Mutex);
        if(*(arguments->dirty)){
            
            memcpy(transferBuffer, arguments->transmissionBuffer, SPI_LEN);
            if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                perror("MESSAGE FAILURE\n");
            }
            arguments->targetingInformation->last_horizontal_position = (uint16_t)receiveBuffer[0] | ((uint16_t)receiveBuffer[1] << 8);
            arguments->targetingInformation->last_vertical_position = (uint16_t)receiveBuffer[2] | ((uint16_t)receiveBuffer[3] << 8);

            printf("TRASMITTED:%X, %X, %X, %X\n", transferBuffer[0], transferBuffer[1], transferBuffer[2], transferBuffer[3]);
            printf("RECIEVED:%X, %X, %X, %X\n", receiveBuffer[0], receiveBuffer[1], receiveBuffer[2], receiveBuffer[3]);
            printf("DECODED:horizontal=%u vertical=%u\n", arguments->targetingInformation->last_horizontal_position,
                 arguments->targetingInformation->last_vertical_position);
            *(arguments->dirty) = 0;
            pthread_cond_signal(&arguments->cond);
        }
        pthread_mutex_unlock(&arguments->SPI_Buffer_Mutex);

    }
    close(fd);
    return 0;
}

int sendMessage(struct SPIArguments *arguments, uint8_t msg[SPI_LEN], int *dirty){
    
    pthread_mutex_lock(&arguments->SPI_Buffer_Mutex);
    
    while(*dirty){
        pthread_cond_wait(&arguments->cond, &arguments->SPI_Buffer_Mutex);
    }

    memcpy(arguments->transmissionBuffer, msg, SPI_LEN);
    *dirty = 1;
    pthread_mutex_unlock(&(arguments->SPI_Buffer_Mutex));
    return 1;
}

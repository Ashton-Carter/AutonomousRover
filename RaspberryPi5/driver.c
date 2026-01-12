
#include "VideoUplink/uplinkServer.h"
#include "VideoCapture/videoCapture.h"
#include "globals.h"
#include "STM32Communication/SPIConnection.h"

#include <sys/shm.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>



struct videoFrame{
    size_t len;
    unsigned char data[];
};


int main(){
    pthread_t video_stream_t;
    pthread_t video_capture_t;
    pthread_t spi_connection_t;

    int sharedMem = shmget(IPC_PRIVATE,  TOTALSHAREDIMAGEMEMORY, IPC_CREAT | 0600);
    char *sharedMemory = shmat(sharedMem, NULL, 0);
    if (sharedMemory == (void *)-1) {
        printf("ERROR IN DRIVER WITH SHAMAT");
        perror("shmat");
        exit(1);
    }

    pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;
    struct serverAttributes atrs = {
        sharedMem,
        &memoryMutex,
        9000
    };

    struct videoCaptureAttributes videoAtrs = {
        sharedMem,
        &memoryMutex
    };
    int drt = 0;
    int *spiDirty = &drt;
    pthread_mutex_t spiMutex = PTHREAD_MUTEX_INITIALIZER;
    struct SPIArguments spiArgs = {
        spiDirty,
        &spiMutex,
        {0x00, 0x00, 0x00, 0x00}
    };

    pthread_create(&video_capture_t, NULL, start_video_capture, &videoAtrs);
    pthread_create(&video_stream_t, NULL, run_video_server, &atrs);
    pthread_create(&spi_connection_t, NULL, SPIHandler, &spiArgs);

    sleep(5);
    sendMessage(&spiMutex, spiArgs.transmissionBuffer, (uint8_t[]){0x10, 0x13, 0x38, 0xFF}, spiDirty);
    sleep(5);
    sendMessage(&spiMutex, spiArgs.transmissionBuffer, (uint8_t[]){0x11, 0x13, 0x38, 0xFF}, spiDirty);
    sleep(5);
    sendMessage(&spiMutex, spiArgs.transmissionBuffer, (uint8_t[]){0x12, 0x13, 0x38, 0xFF}, spiDirty);
    sleep(5);
    sendMessage(&spiMutex, spiArgs.transmissionBuffer, (uint8_t[]){0x13, 0x13, 0x38, 0xFF}, spiDirty);
    sleep(5);
    
    if(shmctl(sharedMem, IPC_RMID, NULL) == -1) {
        printf("ERROR MARKING MEMORY FOR DELETION(Driver)");
    }

}


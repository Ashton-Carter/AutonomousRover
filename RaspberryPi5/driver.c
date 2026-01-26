
#include "C_Video/VideoUplink/uplinkServer.h"
#include "C_Video/VideoCapture/videoCapture.h"
#include "globals.h"
#include "STM32Communication/SPIConnection.h"
#include "ExternalConnection/ManualControl/manualControl.h"

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
    pthread_t manual_control_t;

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

    struct manualControlArgs manArgs = {
        MANUAL_CONTROL_PORT,
        0,
        {0},
        0,
        0
    };


    pthread_create(&video_capture_t, NULL, start_video_capture, &videoAtrs);
    pthread_create(&video_stream_t, NULL, run_video_server, &atrs);
    pthread_create(&spi_connection_t, NULL, SPIHandler, &spiArgs);
    pthread_create(&manual_control_t, NULL, socket_lifecycle, &manArgs);

    while(1){
        if(manArgs.changed){
            if (manArgs.command == CLOSE){
                break;
            }
            uint8_t msg[SPI_LEN] = {0};
            msg[0] = manArgs.command;
            for(int i = 1; i < SPI_LEN; ++i){
                msg[i] = manArgs.amount[i-1];
            }
            manArgs.changed = 0;
            sendMessage(&spiMutex, spiArgs.transmissionBuffer, msg, spiDirty);
        }
    }
    
    if(shmctl(sharedMem, IPC_RMID, NULL) == -1) {
        printf("ERROR MARKING MEMORY FOR DELETION(Driver)");
    }

}


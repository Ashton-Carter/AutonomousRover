#include <stdio.h>              // Provides FILE, fread, printf, popen, pclose
#include <stdlib.h>             // Provides perror and general utilities
#include <sys/shm.h>
#include <string.h>

#include "videoCapture.h"
#include "../globals.h"

#define STARTCOMMANDLENGTH 80
#define BUFFER_PADDING 20000
#define BUFFER_LENGTH (MAX_IMAGE+BUFFER_PADDING)

void* start_video_capture(void *arg) {               
    char startCommand[STARTCOMMANDLENGTH];
    struct videoCaptureAttributes *artibs = (struct videoCaptureAttributes *) arg;
    if(artibs == NULL){
        perror("ERROR CASTING VOID PTR TO STRUCT");
        return NULL;
    }

    char *memoryPointer = shmat(artibs->memoryPointer, NULL, 0);
    if (memoryPointer == (void *)-1) {
        printf("ERROR IN RUNNING VIDEO CAPTURE");
        perror("shmat");
        return NULL;
    }

    snprintf(startCommand, STARTCOMMANDLENGTH, "rpicam-vid --codec mjpeg --width %i --nopreview -t 0 --height %i --output -", MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);

    FILE* cam = popen(startCommand, "r");

    if (!cam) {                 
        perror("popen");        
        return NULL;
    }


    unsigned char buffer[BUFFER_LENGTH];
    unsigned char readInBuffer[IMAGE_READ_AMOUNT];
    size_t currentPosition = 0;
    unsigned char byte;
    int valid = 0;

    size_t read;
    while ((read = fread(readInBuffer, 1, IMAGE_READ_AMOUNT, cam)) > 0) { // Read 1 byte at a time from camera
        for(int i = 0; i < read; ++i){
            byte = readInBuffer[i];
            buffer[currentPosition++] = byte;

            if(!(currentPosition < BUFFER_LENGTH)){
                printf("BUFFER LIMIT HIT!\n");
                currentPosition = 0;
                valid = 0;
            }

            if (currentPosition >= 2 &&                
                buffer[currentPosition - 2] == 0xFF && 
                buffer[currentPosition - 1] == 0xD8) {
                buffer[0] = 0xFF;
                buffer[1] = 0xD8;
                currentPosition = 2;
                valid = 1;     
            }
            if (currentPosition >= 2 &&                
                buffer[currentPosition - 2] == 0xFF && 
                buffer[currentPosition - 1] == 0xD9) {
                if(!valid){
                    continue;
                }
                valid = 0;   
                writeToMemory(memoryPointer, currentPosition, artibs->memoryMux, buffer);
                currentPosition=0;
            }
        }
    }

    pclose(cam);
    return NULL;
}

int writeToMemory(char *memoryPointer, size_t endingPosition, pthread_mutex_t *memMux, unsigned char *buffer){

    pthread_mutex_lock(memMux);
    memcpy(memoryPointer, &endingPosition, sizeof(size_t));
    memcpy(memoryPointer+sizeof(size_t), buffer, endingPosition);
    pthread_mutex_unlock(memMux);
    return 1;
}

#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H

#include <stddef.h>
#include <pthread.h>

struct videoCaptureAttributes {
    int memoryPointer;
    pthread_mutex_t *memoryMux;
};

int writeToMemory(char *memoryPointer, size_t endingPosition, pthread_mutex_t *memMux, unsigned char *buffer);
void* start_video_capture(void *arg);

#endif
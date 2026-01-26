#ifndef UPLINKSERVER_H
#define UPLINKSERVER_H

#include <pthread.h>
#include <stddef.h>

struct serverAttributes {
    int sharedMemoryPtr;
    pthread_mutex_t *memoryMutex;
    int port;
};

struct accepterArguments {
    int serverID;
    pthread_mutex_t *usersMux;
    int *curUsers;
    int *users;
};

void send_jpeg(struct accepterArguments* arguments, char* sharedMemoryPointer, size_t memorySize, pthread_mutex_t *memoryMutex, unsigned char *memory);
void* run_video_server(void* arg);

#endif //UPLINKSERVER_H
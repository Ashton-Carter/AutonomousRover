#ifndef PYTHONIPC_H
#define PYTHONIPC_H
#include <pthread.h>
#include <string.h>
#include "../globals.h"

struct pythonIPCStruct{
    pthread_mutex_t pythonMutex;
    float x;
    float y;
    uint8_t changed;
};

// Do not add padding
#pragma pack(push, 1)
struct pythonIPCMessage {
    float x;
    float y;
};
#pragma pack(pop)



void* start_python_socket(void* args);



#endif
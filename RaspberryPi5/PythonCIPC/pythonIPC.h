#ifndef PYTHONIPC_H
#define PYTHONIPC_H
#include <pthread.h>
#include <string.h>
#include "../globals.h"

struct pythonIPCStruct{
    uint8_t command;
    uint8_t amount[SPI_LEN-1];
    uint8_t changed;
    uint64_t last_update;
};


void* start_python_socket(void* args);

int handlePythonControl(struct pythonIPCStruct *shared, uint8_t msg[SPI_LEN]);


#endif
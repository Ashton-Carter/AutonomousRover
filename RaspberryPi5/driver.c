#include "globals.h"
#include "STM32Communication/SPIConnection.h"
#include "ExternalConnection/ManualControl/manualControl.h"
#include "PythonCIPC/pythonIPC.h"

#include <sys/shm.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


struct threadStatus threadStatus= {
    .manualControl = 0, 
    .manualConnectionStatus = 0, 
    .pythonCVConnectionStatus = 0
};

int main(){
    
    pthread_t spi_connection_t;
    pthread_t manual_control_t;
    pthread_t python_ipc_t;

   
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

    pthread_mutex_t pythonMutex = PTHREAD_MUTEX_INITIALIZER; 
    struct pythonIPCStruct pythonArgs = {
        pythonMutex,
        0,
        0,
        0
    };


    pthread_create(&spi_connection_t, NULL, SPIHandler, &spiArgs);
    pthread_create(&manual_control_t, NULL, socket_lifecycle, &manArgs);
    pthread_create(&python_ipc_t, NULL, start_python_socket, &pythonArgs);

    uint8_t msg[SPI_BUFFER][SPI_LEN] = {0};
    uint8_t changed;
    while(1){
        messages = 0;
        if(threadStatus.manualControl){
            int res = handleManualControl(&manArgs, msg);
            if (res < 0){
                printf("SWITCHING TO AUTONOMOUS CONTROL\n");
                threadStatus.manualControl = 0;
                continue;
            }
            messages = res;
        } else {
            int res = handlePythonControl(&pythonArgs, msg);
            messages = res;
        }
        
        for (i = 0; i < changed; ++i) {
            sendMessage(&spiMutex, spiArgs.transmissionBuffer, msg[i], spiDirty);
        }
    }
    

}


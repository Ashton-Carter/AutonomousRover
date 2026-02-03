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
    struct SPIArguments spiArgs = {
        spiDirty,
        .SPI_Buffer_Mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        {0x00, 0x00, 0x00, 0x00}
    };

    struct manualControlArgs manArgs = {
        MANUAL_CONTROL_PORT,
        0,
        {0},
        0
    };

    struct pythonIPCStruct pythonArgs = {
        .pythonMutex = PTHREAD_MUTEX_INITIALIZER,
        0,
        0,
        0
    };


    pthread_create(&spi_connection_t, NULL, SPIHandler, &spiArgs);
    pthread_create(&manual_control_t, NULL, socket_lifecycle, &manArgs);
    pthread_create(&python_ipc_t, NULL, start_python_socket, &pythonArgs);

    uint8_t msg[SPI_BUFFER][SPI_LEN] = {0};
    uint8_t x_command;
    uint8_t y_command;
    float x_offset;
    float y_offset;
    int x_offset_time;
    int y_offset_time;
    int messages;

    while(1){
        messages = 0;
        if(threadStatus.manualControl){
            int res = handleManualControl(&manArgs, msg[0]);
            if (res < 0){
                printf("SWITCHING TO AUTONOMOUS CONTROL\n");
                threadStatus.manualControl = 0;
                continue;
            }
            messages = res;
        } else {
            pthread_mutex_lock(&pythonArgs.pythonMutex);
            if(pythonArgs.changed){
                x_offset = pythonArgs.x - 0.5;
                y_offset = -1 * (pythonArgs.y - 0.5);
                x_command = CAMERA_RIGHT;
                y_command = CAMERA_UP;
                if(x_offset < 0){
                    x_command = CAMERA_LEFT;
                    x_offset *= -1;
                }
                if(y_offset < 0){
                    y_command = CAMERA_DOWN;
                    y_offset *= -1;
                }
                x_offset_time = x_offset * 1000;
                y_offset_time = y_offset * 1000;
                msg[0][0] = x_command;
                for(int i = SPI_LEN-1; i>=1; --i){
                    msg[0][SPI_LEN-i] = ((x_offset_time >> ((i-1)*8)) & 0xFF);
                }

                msg[1][0] = y_command;
                for(int i = SPI_LEN-1; i>=1; --i){
                    msg[1][SPI_LEN-i] = ((y_offset_time >> ((i-1)*8)) & 0xFF);
                }
                messages = 2;
                pythonArgs.changed = 0;
            }
            pthread_mutex_unlock(&pythonArgs.pythonMutex);
        }
        
        for (int i = 0; i < messages; ++i) {
            sendMessage(&spiArgs.SPI_Buffer_Mutex, &spiArgs.cond, spiArgs.transmissionBuffer, msg[i], spiDirty);
        }
    }
}


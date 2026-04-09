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

void translateToBuffer(uint8_t buffer[SPI_LEN], uint8_t command, unsigned int time_offset){
    buffer[0] = command;
    for(int i = SPI_LEN-1; i>=1; --i){
        buffer[SPI_LEN-i] = ((time_offset >> ((i-1)*8)) & 0xFF);
    }
}

void scan(targetingInformation *targetingInformation, uint8_t toSPITransmission[SPI_BUFFER][SPI_LEN], int *messagesToSPI){
    if(targetingInformation->last_horizontal_position > HORIZONTAL_MAX_SERVO - (HORIZONTAL_TIME_SCALER/2)){
        targetingInformation->currentScanHorizontal = CAMERA_RIGHT;
    } else if (targetingInformation->last_horizontal_position < HORIZONTAL_MIN_SERVO + (HORIZONTAL_TIME_SCALER/2)){
        targetingInformation->currentScanHorizontal = CAMERA_LEFT;
    }

    if(targetingInformation->last_vertical_position > VERTICAL_MAX_SERVO - (VERTICAL_TIME_SCALER/2)){
        targetingInformation->currentScanVertical = CAMERA_DOWN;
    } else if (targetingInformation->last_vertical_position < VERTICAL_MIN_SERVO + (VERTICAL_TIME_SCALER/2)){
        targetingInformation->currentScanVertical = CAMERA_UP;
    }

    translateToBuffer(toSPITransmission[(*messagesToSPI)++], targetingInformation->currentScanHorizontal, SCAN_AMOUNT);
    translateToBuffer(toSPITransmission[(*messagesToSPI)++], targetingInformation->currentScanVertical, SCAN_AMOUNT);
}

int translateOffsetToControlTimes(struct pythonIPCStruct* pythonArgs, uint8_t* x_command, uint8_t* y_command, int* x_offset_time, int* y_offset_time, targetingInformation *targetingInformation){
    targetingInformation->consequtive_tracking_number = pythonArgs->id;
    if(!targetingInformation->consequtive_tracking_number){
        return 0;
    }
    float x_offset = pythonArgs->x - 0.5f;
    float y_offset = -1.0f * (pythonArgs->y - 0.5f);

    *x_command = CAMERA_RIGHT;
    *y_command = CAMERA_UP;
    if(x_offset < 0){
        *x_command = CAMERA_LEFT;
        x_offset *= -1;
    }
    if(y_offset < 0){
        *y_command = CAMERA_DOWN;
        y_offset *= -1;
    }
    *x_offset_time = (int)(x_offset * HORIZONTAL_TIME_SCALER);
    *y_offset_time = (int)(y_offset * VERTICAL_TIME_SCALER);
    return 1;
}


int main(){
    
    pthread_t spi_connection_t;
    pthread_t manual_control_t;
    pthread_t python_ipc_t;

   

    int drt = 0;
    targetingInformation targetingInformation = {
        .last_vertical_position = 0,
        .last_horizontal_position = 0,
        .consequtive_tracking_number = 0,
        .consequtive_classification_without_target = 0,
        .currentScanHorizontal = CAMERA_LEFT,
        .currentScanVertical = CAMERA_UP
    };
    SPIArguments spiArgs = {
        .trasmissionFreeIndex = 0,
        .recieveFreeIndex = 0,
    
        .transmissionMutex = PTHREAD_MUTEX_INITIALIZER,
        .recieveMutex = PTHREAD_MUTEX_INITIALIZER,
        
        .transmissionBuffer = {0},
        .recieveBuffer = {0}
    };


    manualControlArgs manArgs = {
        .port = MANUAL_CONTROL_PORT,
        .amount = 0,
        .changed = 0,
        .command=  0
    };

    struct pythonIPCStruct pythonArgs = {
        .pythonMutex = PTHREAD_MUTEX_INITIALIZER,
        .changed = 0,
        .id = 0,
        .x = 0,
        .y = 0
    };


    pthread_create(&spi_connection_t, NULL, SPIHandler, &spiArgs);
    pthread_create(&manual_control_t, NULL, socket_lifecycle, &manArgs);
    pthread_create(&python_ipc_t, NULL, start_python_socket, &pythonArgs);

    uint8_t toSPITransmission[SPI_BUFFER][SPI_LEN] = {0};
    uint8_t fromSPIRecieve[SPI_BUFFER][SPI_LEN] = {0};
    int messagesFromSPI = 0;
    int messagesToSPI = 0;

    uint8_t x_command;
    uint8_t y_command;
    float x_offset;
    float y_offset;
    int x_offset_time;
    int y_offset_time;
    

    while(1){
        messagesFromSPI = 0;
        messagesToSPI = 0;
        for(int i = 0; i < SPI_BUFFER; ++i){
            if(recieveMessage(&spiArgs, fromSPIRecieve[i]) < 1){
                break;
            }
            messagesFromSPI++;
        }

        if(threadStatus.manualControl){
            unsigned int timeOffset;
            uint8_t command;
            if (handleManualControl(&manArgs, &command, &timeOffset) < 1){
                continue;
            }
            translateToBuffer(toSPITransmission[messagesToSPI++], command, timeOffset);

        } else {
            pthread_mutex_lock(&pythonArgs.pythonMutex);
            if(pythonArgs.changed){
                if (pythonArgs.id){
                    targetingInformation.consequtive_classification_without_target = 0;
                } else {
                    targetingInformation.consequtive_classification_without_target++;
                }

                if (targetingInformation.consequtive_classification_without_target > 10){
                    scan(&targetingInformation, toSPITransmission, &messagesToSPI);
    
                }

                if(translateOffsetToControlTimes(&pythonArgs, &x_command, &y_command, &x_offset_time, &y_offset_time, &targetingInformation)){
                    translateToBuffer(toSPITransmission[messagesToSPI++], x_command, x_offset_time);
                    translateToBuffer(toSPITransmission[messagesToSPI++], y_command, y_offset_time);
                }

                if(targetingInformation.consequtive_tracking_number > 3){
                    translateToBuffer(toSPITransmission[messagesToSPI++], FIRE, FIRE_LENGTH);
                    
                }

                pythonArgs.changed = 0;
            }

            pthread_mutex_unlock(&pythonArgs.pythonMutex);
        }
        
        for (int i = messagesToSPI-1; i >= 0; --i) {
            sendMessage(&spiArgs, toSPITransmission[i]);
        }
    }
}

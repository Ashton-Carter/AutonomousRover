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

void inputSpiMessages(uint8_t fromSpi[SPI_BUFFER][SPI_LEN], int messages, targetingInformation* targetingInformation){
    for(int i = 0; i < messages; ++i){
        //Remove once we have SPI framing
        targetingInformation->last_horizontal_position = (uint16_t)fromSpi[i][0]<<8 | ((uint16_t)fromSpi[i][1]);
        targetingInformation->last_vertical_position = (uint16_t)fromSpi[i][2]<<8 | ((uint16_t)fromSpi[i][3]);
        targetingInformation->distance = 
        (uint32_t)(fromSpi[i][4] << 24)|
        (uint32_t)(fromSpi[i][5] << 16)|
        (uint32_t)(fromSpi[i][6] << 8)|
        (uint32_t)fromSpi[i][7];
    }
}

void moveAndResetMovementScan(targetingInformation *targetingInformation, uint8_t toSPITransmission[SPI_BUFFER][SPI_LEN], int* messagesToSPI){
    int lowEnd = -1;
    int highEnd = -1;
    int bestLow = -1;
    int bestHigh = -1;
    for(int i = 0; i < POSSIBLE_ORIENTATIONS; ++i){
        if(targetingInformation->vehicleDistanceScan[i] < DISTANCE_THRESHOLD){
            if(highEnd-lowEnd > bestHigh-bestLow){
                bestHigh = highEnd;
                bestLow = lowEnd;
            }
            highEnd = -1;
            lowEnd = -1;
            continue;
        }
        if(lowEnd < 0){
            lowEnd = i;
        }
        highEnd = i;
    }
    if(highEnd-lowEnd > bestHigh-bestLow){
        bestHigh = highEnd;
        bestLow = lowEnd;
    }
    
    int turnAmount = (bestHigh - ((bestHigh-bestLow)/2) ) * VEHICLE_SCAN_AMOUNT;
    printf("MOVING TO LOCATION:%i BY TURNING:%i\n", bestHigh - ((bestHigh-bestLow)/2), turnAmount);
    translateToBuffer(toSPITransmission[(*messagesToSPI)++], LEFT, turnAmount);
    targetingInformation->currentVehicleScanMode=NOT_SCANNING;
    targetingInformation->vehicleDistanceIndex=0;
    targetingInformation->lastVehicleScan = targetingInformation->consequtive_classification_without_target;
}

void findTarget(targetingInformation *targetingInformation, uint8_t toSPITransmission[SPI_BUFFER][SPI_LEN], int* messagesToSPI){
    scan(targetingInformation, toSPITransmission, messagesToSPI);
    if(((targetingInformation->consequtive_classification_without_target - targetingInformation->lastVehicleScan) > 10)){
        switch (targetingInformation->currentVehicleScanMode)
        {
        case NOT_SCANNING:
            targetingInformation->currentVehicleScanMode = FIND_OBSTACLE;
            printf("MOVING INTO FINDING OBSTACE\n");
        case FIND_OBSTACLE:
            if(targetingInformation->distance > DISTANCE_THRESHOLD){
                translateToBuffer(toSPITransmission[(*messagesToSPI)++], FORWARD, FORWARD_MOVE_AMOUNT);
            } else {
                targetingInformation->currentVehicleScanMode = FIND_OPEN_AREA;
            }
            break;
        case FIND_OPEN_AREA:
            targetingInformation->scanningThrottle++;
            if(targetingInformation->scanningThrottle % SCANNING_THROTTLE_AMOUNT){
                return;
            }
            printf("LOCATION: %i, DISTANCE: %i\n", targetingInformation->vehicleDistanceIndex, targetingInformation->distance);
            if(targetingInformation->vehicleDistanceIndex > 0){
                printf("STARTING FIND OPEN AREA\n");
                targetingInformation->vehicleDistanceScan[targetingInformation->vehicleDistanceIndex-1] 
                = targetingInformation->distance;
            }
            targetingInformation->vehicleDistanceIndex++;
            if(targetingInformation->vehicleDistanceIndex >= POSSIBLE_ORIENTATIONS){
                moveAndResetMovementScan(targetingInformation, toSPITransmission, messagesToSPI);
                printf("OPEN AREA LOCATED\n");
                return;
            }
            translateToBuffer(toSPITransmission[(*messagesToSPI)++], LEFT, VEHICLE_SCAN_AMOUNT);
        }
    }
}


void resetScanningInformation(targetingInformation* targetingInformation){
    targetingInformation->currentVehicleScanMode=NOT_SCANNING;
    targetingInformation->vehicleDistanceIndex=0;
    targetingInformation->lastVehicleScan=0;
    targetingInformation->consequtive_classification_without_target = 0;
    targetingInformation->scanningThrottle = 0;
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
        .distance = 0,
        .currentScanVertical = CAMERA_UP,
        .vehicleDistanceIndex = 0,
        .vehicleDistanceScan = {0},
        .lastVehicleScan = 0,
        .currentVehicleScanMode = NOT_SCANNING,
        .scanningThrottle = 0
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
            if(recieveMessage(&spiArgs, fromSPIRecieve[i]) < 0){
                break;
            }
            messagesFromSPI++; 
        }

        if(messagesFromSPI > 0){
            inputSpiMessages(fromSPIRecieve, messagesFromSPI, &targetingInformation);
            // printf("HPWM:%i, VPWM:%i, DISTANCE(in):%i\n", 
            //     targetingInformation.last_horizontal_position,
            //     targetingInformation.last_vertical_position,
            //     targetingInformation.distance
            // );
        }
        if(threadStatus.manualControl){
            targetingInformation.vehicleDistanceIndex = 0;
            unsigned int timeOffset;
            uint8_t command;
            if (handleManualControl(&manArgs, &command, &timeOffset) < 1){
                continue;
            }

            translateToBuffer(toSPITransmission[messagesToSPI++], command, timeOffset);
            resetScanningInformation(&targetingInformation);

        } else {
            pthread_mutex_lock(&pythonArgs.pythonMutex);
            if(pythonArgs.changed){
                if (pythonArgs.id){
                    resetScanningInformation(&targetingInformation);
                } else {
                    targetingInformation.consequtive_classification_without_target++;
                }


                if (targetingInformation.consequtive_classification_without_target > 10 && (targetingInformation.consequtive_classification_without_target % SCAN_CYCLE == 0)){
                    findTarget(&targetingInformation, toSPITransmission, &messagesToSPI);
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

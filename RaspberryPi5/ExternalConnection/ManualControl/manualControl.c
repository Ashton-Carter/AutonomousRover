#include "manualControl.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void *socket_lifecycle(void *arg) {

    manualControlArgs *args = (manualControlArgs*) arg;
    int socketInt = -1;
    int retries = 0;

    while(socketInt < 0){
        socketInt = socket(AF_INET, SOCK_STREAM, 0);
        if (socketInt >= 0){
            break;
        }

        if (retries > 2) {
            printf("Retry Limit Hit, Exiting...\n");
            return NULL;
        }

        printf("Failed to create socket, retrying\n");
        retries += 1;
        sleep(3);
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(args->port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int bindResult = bind(socketInt, (struct sockaddr *)&address, sizeof(address));
    if(bindResult == -1){
        perror("manual control bind");
        return NULL;
    }

    int clientInt;

    while(1){
        int listenResult = listen(socketInt, 1);
        if(listenResult == -1){
            printf("Error occured listening for clents\n");
            continue;
        }

        clientInt = accept(socketInt, NULL, NULL);
        if (clientInt < 0) {
            printf("Could not accept client connection\n");
            continue;
        }

        threadStatus.manualControl = 1;
        printf("MANUAL CONTROL STARTING\n");

        char buf[1024];
        int n;
        while(1){
            n = (int)recv(clientInt, buf, sizeof(buf) - 1, 0);
            if(n == -1){
                printf("Error Occured Recieveing Message From Client\n");
                threadStatus.manualControl = 0;
                close(clientInt);
                break;
            } else if (n==0) {
                printf("Client Disconnected\n");
                threadStatus.manualControl = 0;
                close(clientInt);
                break;
            }

            char command = buf[0];
            if(command == 's'){
                args->command = CLOSE;
                args->changed = 1;
                threadStatus.manualControl = 0;
                close(clientInt);
                printf("SWITCHING TO AUTONOMOUS CONTROL\n");
                break;
            } 
            if (command == 'm') {
                if(buf[1]=='F'){
                    args->command = FORWARD;
                }
                if(buf[1]=='L'){
                    args->command = LEFT;
                }
                if(buf[1]=='R'){
                    args->command = RIGHT;
                }
                if(buf[1]=='B'){
                    args->command = BACK;
                }
            }
            if (command == 'f'){
                args->command = FIRE;
            }
            if (command == 'l'){
                if(buf[1]=='U'){
                    args->command = CAMERA_UP;
                }
                if(buf[1]=='L'){
                    args->command = CAMERA_LEFT;
                }
                if(buf[1]=='R'){
                    args->command = CAMERA_RIGHT;
                }
                if(buf[1]=='D'){
                    args->command = CAMERA_DOWN;
                }
            }
            args->amount = 50;
            args->changed = 1;
        }

    }

    
    close(socketInt);

    return NULL;
}



uint64_t now_ms(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

int handleManualControl(manualControlArgs *args, uint8_t *command, unsigned int *timeOffset){
    if(args->changed){
        if (args->command == CLOSE){
            args->changed = 0;
            return -1;
        }

        *command = args->command;
        *timeOffset = args->amount;
        
        args->changed = 0;
        return 1;
    }
    return 0;
}
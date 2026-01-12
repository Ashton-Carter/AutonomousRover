#include "manualControl.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>


int socket_lifecycle(int port) {
    int socketInt = -1;
    int retries = 0;

    while(socketInt < 0){
        socketInt = socket(AF_INET, SOCK_STREAM, 0);
        if (socketInt >= 0){
            break;
        }

        if (retries > 2) {
            printf("Retry Limit Hit, Exiting...\n");
            return -1;
        }

        printf("Failed to create socket, retrying\n");
        retries += 1;
        sleep(3);
    }
    printf("Created Socket\n");

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int bindResult = bind(socketInt, (struct sockaddr *)&address, sizeof(address));
    if(bindResult == -1){
        printf("Error occured binding to port(%i)\n", port);
        return -1;
    }

    int listenResult = listen(socketInt, 1);
    if(listenResult == -1){
        printf("Error occured listening for clents\n");
        return -1;
    }

    int clientInt = accept(socketInt, NULL, NULL);
    if (clientInt < 0) {
        printf("Could not accept client connection\n");
        return -1;
    }

    char buf[1024];
    int n;
    while(1){
        n = (int)recv(clientInt, buf, sizeof(buf) - 1, 0);
        if(n == -1){
            printf("Error Occured\n");
            return -1;
        } else if (n==0) {
            printf("Client Disconnected\n");
            return 0;
        }

        printf("%s", buf);
        char command = buf[0];
        if(command == 's'){
            shutdown_socket();
            break;
        } else if (command == 'm') {
            move(buf[1]);
        }
    }

    close(clientInt);
    close(socketInt);

    return 0;
}

int move(char direction){
    printf("Moving:%c\n", direction);
    return 0;
}

int shutdown_socket(){
    return 0;
}
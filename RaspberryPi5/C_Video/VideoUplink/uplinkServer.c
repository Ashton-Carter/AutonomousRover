#include "uplinkServer.h"
#include "../../globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <pthread.h>

#define BOUNDARY "frame"

void send_jpeg(struct accepterArguments* arguments, char* sharedMemoryPointer, size_t memorySize, pthread_mutex_t *memoryMutex, unsigned char *memory) {
    int brokenConnections[10];
    int brokenConnectionsAmt = 0;
    size_t len;
    pthread_mutex_lock(memoryMutex);
    memcpy(&len, sharedMemoryPointer, sizeof(size_t));
    memcpy(memory, sharedMemoryPointer + sizeof(size_t), len);
    pthread_mutex_unlock(memoryMutex);

    pthread_mutex_lock(arguments->usersMux);
    for(int i = 0; i < *(arguments->curUsers); ++i){
        if (dprintf(arguments->users[i],
            "--%s\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %zu\r\n\r\n",
            BOUNDARY, len) < 0){
                printf("ERROR formating HEADER");
                brokenConnections[brokenConnectionsAmt] = arguments->users[i];
                brokenConnectionsAmt += 1;
                continue;

        }
            

        ssize_t written = write(arguments->users[i], memory, len);
        if (written < 0){
            printf("ERROR WRITING PICTURE");
            brokenConnections[brokenConnectionsAmt] = arguments->users[i];
            brokenConnectionsAmt += 1;
            continue;
        }

        if (write(arguments->users[i], "\r\n", 2) < 0){
            printf("ERROR WRITING ENDING");
            brokenConnections[brokenConnectionsAmt] = arguments->users[i];
            brokenConnectionsAmt += 1;
            continue;
        }
    }
    if (brokenConnectionsAmt > 0){
        for(int i = 0; i < brokenConnectionsAmt; ++i){
            int brokenID = brokenConnections[i];
            int brokenIndex;
            for(int j = 0; j < *(arguments->curUsers); ++j){
                if(brokenID == arguments->users[j]){
                    brokenIndex = j;
                    break;
                }
            }

            close(brokenID);
            int last = *(arguments->curUsers) - 1;
            arguments->users[brokenIndex] = arguments->users[last];
            *(arguments->curUsers) -= 1;
        }
    }
    pthread_mutex_unlock(arguments->usersMux);
}

void* handle_incoming_connections(void *arg){
    printf("ACCEPTER STARTED\n");
    struct accepterArguments* arguments = (struct accepterArguments*) arg;
    while(1){
        if(*(arguments->curUsers) >= 10){
            usleep(100000);
            continue;
        }

        int client_fd = accept(arguments->serverID, NULL, NULL);
        printf("CLIENT ACCPETED\n");
        if(client_fd < 0){
            printf("ERROR ACCEPTING USER\n");
            continue;
        }

        dprintf(client_fd,
            "HTTP/1.1 200 OK\r\n"
            "Cache-Control: no-cache\r\n"
            "Pragma: no-cache\r\n"
            "Connection: close\r\n"
            "Content-Type: multipart/x-mixed-replace; boundary=%s\r\n\r\n",
            BOUNDARY);
        
        pthread_mutex_lock(arguments->usersMux);
        int currentIndex = * (arguments->curUsers);
        arguments->users[currentIndex] = client_fd;
        *(arguments->curUsers) += 1;
        pthread_mutex_unlock(arguments->usersMux);
    }
    
}

void* run_video_server(void* arg) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct serverAttributes* attributes = (struct serverAttributes*) arg;

    char *sharedMemoryPointer = shmat(attributes->sharedMemoryPtr, NULL, SHM_RDONLY);
    if (sharedMemoryPointer == (void *)-1) {
        printf("ERROR IN RUNNING VIDEO SERVER");
        perror("shmat");
        return NULL;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(attributes->port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);

    printf("MJPEG server listening on port %d\n", attributes->port);
    pthread_mutex_t usersMux = PTHREAD_MUTEX_INITIALIZER;
    int *users = malloc(10 * sizeof(int));
    int usrcnt = 0;
    int *userCount = &usrcnt;

    struct accepterArguments acpAtr = {
        server_fd,
        &usersMux,
        userCount,
        users
    };

    pthread_t accepter;
    if(pthread_create(&accepter, NULL, handle_incoming_connections, &acpAtr) != 0){
        printf("ERROR CREATING THREAD!");

    }

    unsigned char *memory = malloc(MAX_IMAGE);
    while (1) {
        send_jpeg(&acpAtr, sharedMemoryPointer, MAX_IMAGE, attributes->memoryMutex, memory);
        usleep(100000);
    }
    free(memory);
    pthread_mutex_lock(&usersMux);
    for(int i = 0; i < *userCount; ++i){
        close(users[i]);
    }
    pthread_mutex_unlock(&usersMux);
    close(server_fd);
    free(users);

    return NULL;
}

#include "pythonIPC.h"

#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>


void* start_python_socket(void* args){
    struct pythonIPCStruct *arguments = (struct pythonIPCStruct *) args;

    int server_fd;
    struct sockaddr_un addr;

    server_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return NULL;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_DOMAIN_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(addr.sun_path);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("python bind");
        close(server_fd);
        return NULL;
    }

    while(1){
        if (listen(server_fd, 1) == -1) {
            perror("listen");
            continue;
        }

        printf("UNIX SOCKET listening on %s\n", addr.sun_path);

        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        while(1) {
            struct pythonIPCMessage message;
            int n;
            while(1){
                n = recv(client_fd, &message, sizeof(message), 0);
                if(n == -1){
                    printf("Error Occured Recieveing Message From Python\n");
                    break;
                } else if (n==0) {
                    printf("Python Disconnected\n");
                    break;
                }
                pthread_mutex_lock(&arguments->pythonMutex);
                arguments->x = message.x;
                arguments->y = message.y;
                arguments->changed = 1;
                pthread_mutex_unlock(&arguments->pythonMutex);
                
            }
        }

    }

}

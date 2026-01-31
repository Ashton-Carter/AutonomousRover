#include "pythonIPC.h"

#include <sys/socket.h>
#include <stdio.h>

void* start_python_socket(void* args){
    struct pythonIPCStruct *arguments = (struct pythonIPCStruct *) args;

    int server_fd;
    struct sockaddr_un addr;

    server_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(addr.sun_path);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return;
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
            char buf[1024];
            int n;
            while(1){
                n = (int)recv(clientInt, buf, sizeof(buf) - 1, 0);
                if(n == -1){
                    printf("Error Occured Recieveing Message From Python\n");
                    break;
                } else if (n==0) {
                    printf("Python Disconnected\n");
                    break;
                }

                char command = buf[0];
                if(command == 's'){
                    args->command = CLOSE;
                    args->changed = 1;
                    threadStatus.manualControl = 0;
                    sleep(5);
                    break;
                } else if (command == 'm') {
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
                    memcpy(args->amount, (uint8_t[]){0x00, 0x00, 0x64}, 3);
                    args->changed = 1;
                    args->last_update = now_ms();
                }
            }
        }

    }

}

int handlePythonControl(struct pythonIPCStruct *shared, uint8_t msg[SPI_LEN]){

    return 1;
}
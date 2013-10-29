#include "server.h"

namespace
{
    #define BACKLOG 10
    #define BUFFER_SIZE 256

    vector<pid_t> pid_list;
}

int Recieve(int remote_socket);

extern int quit_flag;

void TcpServer(int server_socket)
{
    if (listen(server_socket, BACKLOG) == -1) {
        perror("listen()");
        return;
    }

    while (quit_flag == 0) {
        int remote_socket = accept(server_socket, NULL, NULL);

        if (remote_socket == -1) {
            perror("accept()");
            continue;
        }

        pid_t p = fork();

        switch (p) {
            case -1:
                perror("fork()");
                break;
            case 0:
                if (Recieve(remote_socket) == -1) {
                    perror("Recieve()");
                }
                close(remote_socket);
                exit(0);
            default:
                pid_list.push_back(p);
        }
    }

    for (int i = 0; i < pid_list.size(); i++) {
        waitpid(pid_list[i], NULL, 0);
    }

    pid_list.clear();
}

int Recieve(int remote_socket)
{
    int totalBytesRecieved = 0;
    int bytes_read;
    int file_size;
    char buffer[BUFFER_SIZE];
    FILE *fd;

    // Recieve filename
    bytes_read = recv(remote_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read < 1)
        return -1;

    // Create file
    fd = fopen(buffer, "w");
    if (fd == NULL)
        return -1;

    // Recieve file size
    bytes_read = recv(remote_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read < 1)
        return -1;

    file_size = atoi(buffer);

    // Recieve file data
    while (totalBytesRecieved < file_size) {
        bytes_read = recv(remote_socket, buffer, BUFFER_SIZE, 0);

        if (sockatmark(remote_socket) == 1) {
            printf("recieved %d bytes\n", totalBytesRecieved);

            char cbuf;
            if (recv(remote_socket, &cbuf, 1, MSG_OOB) < 1) {
                perror("recv() OOB error");
            }

            if (bytes_read < 1)
                continue;
        }

        if (bytes_read == 0)
            break;

        if (bytes_read == -1) {
            fclose(fd);
            return -1;
        }

        fwrite(buffer, sizeof(unsigned char), bytes_read, fd);
        totalBytesRecieved += bytes_read;
    }

    printf("Total bytes recieved: %d\n", totalBytesRecieved);
    fclose(fd);
    return 0;
}


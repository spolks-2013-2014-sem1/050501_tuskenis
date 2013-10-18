#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include "../lib-spolks/tcp_wrap.h"
#include "../lib-spolks/utils.h"

#define BUFFER_SIZE 256

int quit_flag = 0;

void sig_handler(int i)
{
    quit_flag = 1;
}

void urg_handler(int i)
{
    //dummy
}

int recv_file(int remote_socket)
{
    int totalBytesRecieved = 0;
    int bytes_read;
    int file_size;
    char buffer[BUFFER_SIZE];
    FILE *fd;

    // Recieve filename
    bytes_read = recv(remote_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 1)
        return -1;
    buffer[bytes_read] = '\0';

    // Create file

    fd = fopen(buffer, "w");
    if (fd == NULL)
        return -1;

    // Recieve file size
    bytes_read = recv(remote_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 1)
        return -1;
    buffer[bytes_read] = '\0';
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

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("\n\tUsage: %s <host> <port>\n\n", argv[0]);
        return 0;
    }

    char *host = argv[1];
    char *port = argv[2];

    set_sig_handler(SIGINT, sig_handler);
    set_sig_handler(SIGURG, urg_handler);

    int server_socket = create_tcp_server(host, atoi(port), 10);

    if (server_socket == -1) {
        perror("create_tcp_server() errno");
        return 0;
    }

    while (quit_flag == 0) {
        int remote_socket = accept(server_socket, NULL, NULL);

        if (remote_socket == -1) {
            perror("accept() errno");
            continue;
        }

        if (fcntl(remote_socket, F_SETOWN, getpid()) == -1)
            perror("fcntl() error");

        if (recv_file(remote_socket) == -1)
            perror("recv_file() errno");

        close(remote_socket);
    }

    close(server_socket);
    return 0;
}

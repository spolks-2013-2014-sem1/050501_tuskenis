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
#include "../lib-spolks/sockets_wrap.h"
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

int recv_file_tcp(int remote_socket)
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

int recv_file_udp(int remote_socket)
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

	set_socket_timeout(remote_socket, 15);

    // Recieve file data
    while (totalBytesRecieved < file_size) {
		int sockaddr_len = sizeof(struct sockaddr);
		struct sockaddr addr;

		bytes_read = recvfrom(remote_socket, buffer, BUFFER_SIZE, 0, &addr, &sockaddr_len);

        if (bytes_read <= 0) {
            fclose(fd);
            return -1;
        }

		// Send reply
		sendto(remote_socket, "x", 1, 0, &addr, sockaddr_len);

        totalBytesRecieved += bytes_read;
        fwrite(buffer, sizeof(unsigned char), bytes_read, fd);
    }

	set_socket_timeout(remote_socket, 0);

    printf("Total bytes recieved: %d\n", totalBytesRecieved);
    fclose(fd);
    return 0;
}


int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("\n\tUsage: %s <host> <port> <tcp|udp>\n\n", argv[0]);
        return 0;
    }

    char *host = argv[1];
    char *port = argv[2];
    char *prot = argv[3];

    set_sig_handler(SIGINT, sig_handler);
    set_sig_handler(SIGURG, urg_handler);

    int server_socket = create_socket(prot);

    if (server_socket == -1) {
        perror("create_socket() error");
        return 0;
    }

    if (bind_socket(server_socket, host, atoi(port)) == -1) {
        perror("bind_socket() error");
        close(server_socket);
        return 0;
    }

    if (!strcmp(prot, "tcp") && listen(server_socket, 10) == -1) {
        perror("listen() error");
        close(server_socket);
        return 0;
    }
    // UDP mode
    if (!strcmp(prot, "udp")) {

        while (quit_flag == 0) {
            if (recv_file_udp(server_socket) == -1) {
                perror("recv_file() error");
            }
        }

        close(server_socket);
        return 0;
    }
    // TCP mode
    while (quit_flag == 0) {
        int remote_socket = accept(server_socket, NULL, NULL);

        if (remote_socket == -1) {
            perror("accept() error");
            continue;
        }

        if (fcntl(remote_socket, F_SETOWN, getpid()) == -1)
            perror("fcntl() error");

        if (recv_file_tcp(remote_socket) == -1)
            perror("recv_file() error");

        close(remote_socket);
    }

    close(server_socket);
    return 0;
}

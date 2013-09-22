#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "../lib-spolks/tcp_wrap.h"

#define BUFFER_SIZE 6
unsigned char buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("\n Usage: stcps host port");
        printf("\n    e.g.: ./stcps 127.0.0.1 6677\n\n");
        return 0;
    }

    char *host = argv[1];
    char *port = argv[2];

    int socket_descriptor = create_tcp_server(host, atoi(port), 1);

	if(socket_descriptor == -1)
	{
		perror("create_tcp_server() error");
		return 0;
	}

    printf("Awaiting connection...\n");

    int accepted_socket;

    do {
        accepted_socket = accept(socket_descriptor, NULL, NULL);
    }
    while (accepted_socket == -1);

    printf("Connected.\n");

    int bytes_read = 0;

    while (1) {
        bytes_read = recv(accepted_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_read <= 0) {
            printf("Disconnected.\n");
            break;
        }

        send(accepted_socket, buffer, bytes_read, 0);
    }

    close(socket_descriptor);
    return 0;
}

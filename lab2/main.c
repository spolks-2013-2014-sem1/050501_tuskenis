#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 6
unsigned char buffer[BUFFER_SIZE];

int tcp_socket()
{
    int socket_descriptor = 0;
    struct protoent *protocol = (struct protoent *) getprotobyname("tcp");

    if (protocol == NULL)
        return -1;

    socket_descriptor = socket(PF_INET, SOCK_STREAM, protocol->p_proto);

    return socket_descriptor;
}

int tcp_bind(int socket_descriptor, char *ip_address, unsigned short port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (!inet_aton(ip_address, &addr.sin_addr))
        return -1;

    return bind(socket_descriptor, (struct sockaddr *) &addr,
                sizeof(addr));
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("\n Usage: stcps host port");
        printf("\n    e.g.: ./stcps 127.0.0.1 6677\n\n");
        return 0;
    }

    int socket_descriptor = tcp_socket();

    if (socket_descriptor == -1) {
        printf("Error: creating socket failed.\n");
        return 0;
    }

    if (tcp_bind(socket_descriptor, argv[1], atoi(argv[2])) == -1) {
        printf("Error: binding socket failed.\n");
        close(socket_descriptor);
        return 0;
    }

    if (listen(socket_descriptor, 1) == -1) {
        printf("Error: setting socket to listening state failed.\n");
        close(socket_descriptor);
        return 0;
    }

    printf("Awaiting connection...\n");

    int accepted_socket;

    do
        accepted_socket = accept(socket_descriptor, NULL, NULL);
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

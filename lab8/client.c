#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <error.h>
#include <signal.h>
#include "../lib-spolks/sockets_wrap.h"
#include "../lib-spolks/utils.h"

#define BUFFER_SIZE 256

int send_file_tcp(char *filepath, int socket_descriptor)
{
    int totalBytesSent = 0;
    char buffer[BUFFER_SIZE];
    FILE *fd;
    int file_size = fsize(filepath);
    int bytes_read;

    fd = fopen(filepath, "r");

    if (fd == NULL || file_size < 1)
        return -1;

    // Send filename
    char *filename = parse_filename(filepath);
    strcpy(buffer, filename);
    send(socket_descriptor, buffer, BUFFER_SIZE, 0);
    free(filename);
    // Send file size
    itoa(file_size, buffer);
    send(socket_descriptor, buffer, BUFFER_SIZE, 0);

    // number of file data sending iterations between sending out-of-band data
    int oob_count = (file_size / BUFFER_SIZE) / 5;

    // Send file data
    int i = 0;                  // file data sending iterations counter
    while (1) {
        bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, fd);

        if (bytes_read <= 0)
            break;

        if (send(socket_descriptor, buffer, bytes_read, 0) == -1) {
            fclose(fd);
            return -1;
        }

        totalBytesSent += bytes_read;

        // Generate out-of-band data
        if (oob_count > 0 && ++i % oob_count == 0) {
            printf("sent %d bytes\n", totalBytesSent);

            if (send(socket_descriptor, "Z", 1, MSG_OOB) == -1)
                perror("send OOB error");
        }

        if (bytes_read < BUFFER_SIZE)
            break;
    }

    printf("Total bytes sent: %d\n", totalBytesSent);
    fclose(fd);
    return 0;
}

int send_file_udp(char *filepath, int socket_descriptor)
{
    int totalBytesSent = 0;
    char buffer[BUFFER_SIZE];
    FILE *fd;
    int file_size = fsize(filepath);
    int bytes_read;

    fd = fopen(filepath, "r");

    if (fd == NULL || file_size < 1)
        return -1;

    // Send filename
    char *filename = parse_filename(filepath);
    strcpy(buffer, filename);
    send(socket_descriptor, buffer, BUFFER_SIZE, 0);
    free(filename);
    // Send file size
    itoa(file_size, buffer);
    send(socket_descriptor, buffer, BUFFER_SIZE, 0);

    // Send file data
    while (1) {
        bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, fd);

        if (bytes_read <= 0)
            break;

        if (send(socket_descriptor, buffer, bytes_read, 0) == -1) {
            fclose(fd);
            return -1;
        }

		// Wait reply
		if (recv(socket_descriptor, buffer, 1, 0) < 1) {
			fclose(fd);
			return -1;
		}

        totalBytesSent += bytes_read;
        printf("%d bytes sent\n", totalBytesSent);
    }

    printf("Total bytes sent: %d\n", totalBytesSent);
    fclose(fd);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 5) {
        printf("\n\tUsage: %s <filepath> <host> <port> <tcp|udp>\n\n",
               argv[0]);
        return 0;
    }

    char *filepath = argv[1];
    char *host = argv[2];
    char *port = argv[3];
    char *protocol = argv[4];
    int (*send_file_routine) (char *, int);

    if (!strcmp(protocol, "tcp"))
        send_file_routine = send_file_tcp;
    else if (!strcmp(protocol, "udp"))
        send_file_routine = send_file_udp;
    else {
        fprintf(stderr, "Invalid protocol specified.\n");
        return 0;
    }

    int socket_descriptor = create_socket(protocol);

    if (socket_descriptor == -1) {
        perror("create_socket() error");
        return 0;
    }

    if (connect_socket(socket_descriptor, host, atoi(port)) == -1) {
        perror("connect_socket() error");
        close(socket_descriptor);
        return 0;
    }

	set_socket_timeout(socket_descriptor, 15);

    if (send_file_routine(filepath, socket_descriptor) == -1)
        perror("send_file() error");

    close(socket_descriptor);
    return 0;
}

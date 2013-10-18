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
#include "../lib-spolks/tcp_wrap.h"
#include "../lib-spolks/utils.h"

#define BUFFER_SIZE 256

int send_file(char *filepath, int socket_descriptor)
{
    int totalBytesSent = 0;
    char buffer[BUFFER_SIZE];
    FILE *fd;
    int file_size = fsize(filepath);
    int bytes_read;

    fd = fopen(filepath, "r");

    if (fd == NULL || file_size < 1)
        return -1;

    itoa(file_size, buffer);

    char *filename = parse_filename(filepath);
    send(socket_descriptor, filename, strlen(filename), 0);     // Send file name
    free(filename);
    send(socket_descriptor, buffer, strlen(buffer), 0); // Send file size

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

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("\n\tUsage: %s <filepath> <host> <port>\n\n", argv[0]);
        return 0;
    }

    char *filepath = argv[1];
    char *host = argv[2];
    char *port = argv[3];

    int socket_descriptor = tcp_connect(host, atoi(port));

    if (socket_descriptor == -1) {
        perror("tcp_connect() errno");
        return 0;
    }

    if (send_file(filepath, socket_descriptor) == -1)
        perror("send_file() errno");

    close(socket_descriptor);
    return 0;
}

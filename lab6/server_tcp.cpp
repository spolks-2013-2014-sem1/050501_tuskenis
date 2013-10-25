#include "server.h"

namespace
{
    #define BACKLOG 10
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];

    map<int, FILE*> mapSockFile;
    map<FILE*, int> mapFileSize;
    map<int, int> mapSockTotalRcv;
}

int Accept(int server_socket);
int Recieve(int socket_descriptor);
int CheckOOB(int socket_descriptor);

extern int quit_flag;
extern int errno;

void TcpServer(int server_socket)
{
    if (listen(server_socket, BACKLOG) == -1) {
        perror("listen()");
        return;
    }

    int dt_size = getdtablesize();

    fd_set sockets;
    fd_set sockets_changed;

    FD_ZERO(&sockets);
    FD_SET(server_socket, &sockets);

    while(quit_flag == 0) {
        memcpy(&sockets_changed, &sockets, sizeof(sockets_changed));

        if (select(dt_size, &sockets_changed, NULL, NULL, NULL) == -1) {
            perror("select()");

            if (quit_flag == 1)
                break;
        }

        if (FD_ISSET(server_socket, &sockets_changed)) {
            int remote_socket = Accept(server_socket);

            if (remote_socket == -1) {
                perror("Accept()");
            } else {
                FD_SET(remote_socket, &sockets);
            }
        }

        for (int sock = 0; sock < dt_size; ++sock) {
            if (sock != server_socket && FD_ISSET(sock, &sockets_changed)) {
                if (Recieve(sock) == -1) {
                    perror("Recieve()");
                }
                if (mapSockFile.find(sock) == mapSockFile.end()) {
                    FD_CLR(sock, &sockets);
                }
            }
        }
    }
}

int Accept(int server_socket)
{
    int remote_socket = accept(server_socket, NULL, NULL);

    if (remote_socket == -1)
        return -1;

    // Recieve file name
    int bytes_read = recv(remote_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read < 1) {
        close(remote_socket);
        return -1;
    }

    buffer[bytes_read] = '\0';

    if (is_file_exists(buffer) == 1) {
        fprintf(stderr, "File is already exists.\n");
        close(remote_socket);
        return -1;
    }

    // Open file
    FILE *fd = fopen(buffer, "w+");

    if (fd == NULL) {
        close(remote_socket);
        return -1;
    }

    // Recieve file size
    bytes_read = recv(remote_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read < 1) {
        close(remote_socket);
        fclose(fd);
        return -1;
    }

    buffer[bytes_read] = '\0';

    int file_size = atoi(buffer);

    // Save associations
    mapSockFile[remote_socket] = fd;
    mapFileSize[fd] = file_size;
    mapSockTotalRcv[remote_socket] = 0;

    // Allow catching URG signal
    if (fcntl(remote_socket, F_SETOWN, getpid()) == -1)
        perror("fcntl()");
    
    return remote_socket;
}

int Recieve(int socket_descriptor)
{
    FILE *fd = mapSockFile[socket_descriptor];
    int file_size = mapFileSize[fd];
    int bytes_read;

    while (quit_flag == 0) {
        bytes_read = recv(socket_descriptor, buffer, BUFFER_SIZE, 0);

        // Check for out-of-band data
        if (CheckOOB(socket_descriptor) == 0 && bytes_read < 1)
            continue;
        else
            break;
    }

    if (bytes_read == -1)
        return -1;

    fwrite(buffer, bytes_read, sizeof(char), fd);

    file_size -= bytes_read;
    mapSockTotalRcv[socket_descriptor] += bytes_read;

    if (file_size < 1) {
        printf("Socket %d; Total bytes recieved: %d\n", socket_descriptor, mapSockTotalRcv[socket_descriptor]);
        close(socket_descriptor);
        fclose(fd);
        mapSockFile.erase(socket_descriptor);
        mapFileSize.erase(fd);
        mapSockTotalRcv.erase(socket_descriptor);
        return 0;
    }

    mapFileSize[fd] = file_size;
    return 0;
}

int CheckOOB(int socket_descriptor)
{
    if (sockatmark(socket_descriptor) == 1) {
        int bytes_recieved = mapSockTotalRcv[socket_descriptor];

        printf("Socket %d; Recieved %d bytes.\n", socket_descriptor, bytes_recieved);

        char cbuf;
        if (recv(socket_descriptor, &cbuf, 1, MSG_OOB) < 1) {
            perror("recv() OOB error");
        }

        return 0;
    }

    return -1;
}


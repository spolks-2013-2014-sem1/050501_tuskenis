#include "server.h"

namespace
{
    #define BACKLOG 10
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];

    struct file_info {
        FILE *fd;
        int file_size;
        int bytes_recieved;
    };

    map<int, struct file_info*> socket_map;
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
                if (socket_map.find(sock) == socket_map.end()) {
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
    int bytes_read = recv(remote_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_read < 1) {
        close(remote_socket);
        return -1;
    }

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
    bytes_read = recv(remote_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_read < 1) {
        close(remote_socket);
        fclose(fd);
        return -1;
    }

    int file_size = atoi(buffer);

    // Save associations
    struct file_info *fi = new struct file_info;
    fi->fd = fd;
    fi->file_size = file_size;
    fi->bytes_recieved = 0;

    socket_map[remote_socket] = fi;

    // Allow catching URG signal
    if (fcntl(remote_socket, F_SETOWN, getpid()) == -1)
        perror("fcntl()");
    
    return remote_socket;
}

int Recieve(int socket_descriptor)
{
    FILE *fd = socket_map[socket_descriptor]->fd;
    int file_size = socket_map[socket_descriptor]->file_size;
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

    socket_map[socket_descriptor]->bytes_recieved += bytes_read;

    if (socket_map[socket_descriptor]->bytes_recieved >= file_size) {
        printf("Socket %d; Total bytes recieved: %d\n", socket_descriptor, socket_map[socket_descriptor]->bytes_recieved);
        close(socket_descriptor);
        fclose(fd);
        delete (struct file_info*)socket_map[socket_descriptor];
        socket_map.erase(socket_descriptor);
        return 0;
    }

    return 0;
}

int CheckOOB(int socket_descriptor)
{
    if (sockatmark(socket_descriptor) == 1) {
        int bytes_recieved = socket_map[socket_descriptor]->bytes_recieved;

        printf("Socket %d; Recieved %d bytes.\n", socket_descriptor, bytes_recieved);

        char cbuf;
        if (recv(socket_descriptor, &cbuf, 1, MSG_OOB) < 1) {
            perror("recv() OOB error");
        }

        return 0;
    }

    return -1;
}


#include "server.h"

namespace
{
    const int BACKLOG = 10;
    const int BUFFER_SIZE = 256;

    struct thread_arg {
        int socket_descriptor;
    };

    vector<pthread_t*> threads;
}

void *RecieveThreadProc(void*);
int recv_file_tcp(int remote_socket);

extern int quit_flag;

void TcpServer(int server_socket)
{
    if (listen(server_socket, BACKLOG) == -1) {
        perror("listen()");
        return;
    }

    while (quit_flag == 0) {

        for (int i = 0; i < threads.size(); i++) {
            if (pthread_tryjoin_np(*threads[i], NULL) == 0) {
                threads.erase(threads.begin() + i);
            }
        }

        struct thread_arg *arg = new struct thread_arg;

        arg->socket_descriptor = accept(server_socket, NULL, NULL);

        if (arg->socket_descriptor == -1) {
            perror("accept()");
            delete arg;
            continue;
        }

        threads.push_back(new pthread_t);

        if (pthread_create(threads[threads.size() - 1], NULL, RecieveThreadProc, arg) != 0) {
            perror("pthread_create()");
            threads.pop_back();
            close(arg->socket_descriptor);
            delete arg;
        }
    }

    for (int i = 0; i < threads.size(); i++) {
        pthread_join(*threads[i], NULL);
    }

    threads.clear();
}

void *RecieveThreadProc(void *arg)
{
    int socket_descriptor = ((struct thread_arg*)arg)->socket_descriptor;

    if (recv_file_tcp(socket_descriptor) == -1) {
        perror("recv_file()");
    }

    close(socket_descriptor);
    delete (struct thread_arg*)arg;
}

int recv_file_tcp(int remote_socket)
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


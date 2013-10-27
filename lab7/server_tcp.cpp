#include "server.h"

namespace
{
    #define BACKLOG 10
    #define BUFFER_SIZE 256

    struct thread_arg {
        int socket_descriptor;
        pthread_t thread_id;
    };

    vector<pthread_t*> threads;
    pthread_mutex_t vector_mutex = PTHREAD_MUTEX_INITIALIZER;
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
        struct thread_arg *arg = new struct thread_arg;

        arg->socket_descriptor = accept(server_socket, NULL, NULL);

        if (arg->socket_descriptor == -1) {
            perror("accept()");
            delete arg;
            continue;
        }

        threads.push_back(&(arg->thread_id));

        if (pthread_create(&(arg->thread_id), NULL, RecieveThreadProc, arg) != 0) {
            perror("pthread_create()");
            threads.pop_back();
            close(arg->socket_descriptor);
            delete arg;
        }
    }

    if (pthread_mutex_lock(&vector_mutex) != 0) {
        perror("pthread_mutex_lock()");
    }

    for (int i = 0; i < threads.size(); i++) {
        pthread_join(*threads[i], NULL);
    }

    threads.clear();
}

void *RecieveThreadProc(void *arg)
{
    pthread_t thread_id = ((struct thread_arg*)arg)->thread_id;
    int socket_descriptor = ((struct thread_arg*)arg)->socket_descriptor;

    if (recv_file_tcp(socket_descriptor) == -1) {
        perror("recv_file()");
    }

    if (pthread_mutex_trylock(&vector_mutex) == 0) {
        for (int i = 0; i < threads.size(); i++) {
            if (*(threads[i]) == thread_id) {
                threads.erase(threads.begin() + i);
                break;
            }
        }
        pthread_mutex_unlock(&vector_mutex);
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


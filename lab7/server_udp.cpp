#include "server.h"

namespace 
{
    const int BUFFER_SIZE = 256;

    char buffer[BUFFER_SIZE];

    struct thread_arg {
        char *buffer;
        int buffer_size;
        long in_addr;
    };

    struct file_info {
        FILE *fd;
        int file_size;
        int bytes_recieved;
        pthread_mutex_t *mutex;
    };

    map<long, struct file_info*> addr_map;
    pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;

    vector<pthread_t*> threads;
}

void *QueryProcessingThread(void *arg);

extern int quit_flag;
extern int errno;

void UdpServer(int server_socket)
{
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(sockaddr_in);    

    while(quit_flag == 0) {

        for (int i = 0; i < threads.size(); i++) {
            if (pthread_tryjoin_np(*threads[i], NULL) == 0) {
                threads.erase(threads.begin() + i);
            }
        }

        int bytes_read = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&address, &addr_len);

        if (bytes_read < 1) {
            perror("recvfrom()");
            continue;
        }

        long in_addr = address.sin_addr.s_addr; // network address

        pthread_mutex_lock(&map_mutex);

        if (addr_map.find(in_addr) == addr_map.end()) {
            pthread_mutex_unlock(&map_mutex);

            if (is_file_exists(buffer) == 1) {
                fprintf(stderr, "File is already exists.\n");
                continue;
            }

            // Open file
            FILE* fd = fopen(buffer, "w+");

            if (fd == NULL)
                continue;

            // Recieve file size
            bytes_read = recv(server_socket, buffer, BUFFER_SIZE, 0);

            if (bytes_read < 1) {
                perror("recv()");
                fclose(fd);
                continue;
            }

            int file_size = atoi(buffer);

            // Save associations
            struct file_info *fi = new file_info;
            fi->fd = fd;
            fi->file_size = file_size;
            fi->bytes_recieved = 0;
            fi->mutex = new pthread_mutex_t;
            pthread_mutex_init(fi->mutex, NULL);

            pthread_mutex_lock(&map_mutex);
            addr_map[in_addr] = fi;
            pthread_mutex_unlock(&map_mutex);

        } else {
            pthread_mutex_unlock(&map_mutex);
            struct thread_arg *arg = new struct thread_arg;

            arg->buffer = new char[bytes_read];
            arg->buffer_size = bytes_read;
            strncpy(arg->buffer, buffer, bytes_read);
            arg->in_addr = in_addr;

            threads.push_back(new pthread_t);

            if (pthread_create(threads[threads.size() - 1], NULL, QueryProcessingThread, arg) != 0) {
                perror("pthread_create()");
                threads.pop_back();
                delete (char*)(arg->buffer);
                delete (struct thread_arg*)arg;
            }

            // Send reply
            sendto(server_socket, "x", 1, 0, (struct sockaddr*)&address, addr_len);
        }

    }

    for (int i = 0; i < threads.size(); i++) {
        pthread_join(*threads[i], NULL);
    }

    threads.clear();
}

void *QueryProcessingThread(void *arg)
{
    // Decode argument
    char *buffer = ((struct thread_arg*)arg)->buffer;
    int buffer_size = ((struct thread_arg*)arg)->buffer_size;
    long in_addr = ((struct thread_arg*)arg)->in_addr;

    pthread_mutex_lock(&map_mutex);
    FILE *fd = addr_map[in_addr]->fd;
    int file_size = addr_map[in_addr]->file_size;
    pthread_mutex_t *mutex = addr_map[in_addr]->mutex;
    pthread_mutex_unlock(&map_mutex);

    pthread_mutex_lock(mutex);

    fwrite(buffer, buffer_size, sizeof(char), fd);

    pthread_mutex_lock(&map_mutex);
    addr_map[in_addr]->bytes_recieved += buffer_size;

    if (addr_map[in_addr]->bytes_recieved >= file_size) {
        _printf("Total bytes recieved: %d\n", addr_map[in_addr]->bytes_recieved);
        fclose(fd);
        pthread_mutex_unlock(mutex);
        pthread_mutex_destroy(mutex);
        delete (pthread_mutex_t*)(addr_map[in_addr]->mutex);
        delete (struct file_info*)addr_map[in_addr];
        addr_map.erase(in_addr);
    } else {
        _printf("Recieved %d bytes.\n", addr_map[in_addr]->bytes_recieved);
        pthread_mutex_unlock(mutex);
    }

    pthread_mutex_unlock(&map_mutex);

    delete (char*)(((struct thread_arg*)arg)->buffer);
    delete (struct thread_arg*)arg;
}


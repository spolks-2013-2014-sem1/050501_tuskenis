#include "server.h"

namespace 
{
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];

    struct file_info {
        FILE *fd;
        int file_size;
        int bytes_recieved;
    };

    map<long, struct file_info*> addr_map;
}

extern int quit_flag;

void UdpServer(int server_socket)
{
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(sockaddr_in);    

    while(quit_flag == 0) {
        int bytes_read = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&address, &addr_len);

        if (bytes_read < 1) {
            perror("recvfrom()");
            continue;
        }

        long in_addr = address.sin_addr.s_addr; // network address

        if (addr_map.find(in_addr) == addr_map.end()) {
            // here buffer contains filename without string-terminator
            buffer[bytes_read] = '\0';

            if (is_file_exists(buffer) == 1) {
                fprintf(stderr, "File is already exists.\n");
                continue;
            }

            // Open file
            FILE* fd = fopen(buffer, "w+");

            if (fd == NULL)
                continue;

            // Recieve file size
            bytes_read = recv(server_socket, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_read < 1) {
                perror("recv()");
                fclose(fd);
                continue;
            }

            buffer[bytes_read] = '\0';

            int file_size = atoi(buffer);

            // Save associations
            struct file_info *fi = new struct file_info;
            fi->fd = fd;
            fi->file_size = file_size;
            fi->bytes_recieved = 0;

            addr_map[in_addr] = fi;

        } else {
            // here buffer contains file data
            FILE *fd = addr_map[in_addr]->fd;
            int file_size = addr_map[in_addr]->file_size;

            fwrite(buffer, bytes_read, sizeof(char), fd);

            addr_map[in_addr]->bytes_recieved += bytes_read;

            // Send reply
            sendto(server_socket, "x", 1, 0, (struct sockaddr*)&address, addr_len);

            if (addr_map[in_addr]->bytes_recieved >= file_size) {
                printf("Total bytes recieved: %d\n", addr_map[in_addr]->bytes_recieved);
                fclose(fd);
                delete (struct file_info*)addr_map[in_addr];
                addr_map.erase(in_addr);
            }
        }
    }
}


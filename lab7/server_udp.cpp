#include "server.h"

namespace 
{
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];

    map<long, FILE*> mapAddrFile;
    map<FILE*, int> mapFileSize;
}

extern int quit_flag;

void UdpServer(int server_socket)
{
    struct sockaddr_in address;
    socklen_t addr_len;    

    while(quit_flag == 0) {
        int bytes_read = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&address, &addr_len);

        if (bytes_read < 1) {
            perror("recvfrom()");
            continue;
        }

        long in_addr = address.sin_addr.s_addr; // network address

        if (mapAddrFile.find(in_addr) == mapAddrFile.end()) {
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
            mapAddrFile[in_addr] = fd;
            mapFileSize[fd] = file_size;    
        } else {
            // here buffer contains file data
            FILE *fd = mapAddrFile[in_addr];
            int file_size = mapFileSize[fd];

            fwrite(buffer, bytes_read, sizeof(char), fd);

            file_size -= bytes_read;

            // Send reply
            sendto(server_socket, "x", 1, 0, (struct sockaddr*)&address, addr_len);

            if (file_size < 1) {
                fclose(fd);
                mapAddrFile.erase(in_addr);
                mapFileSize.erase(fd);
            }

            mapFileSize[fd] = file_size;
        }
    }
}


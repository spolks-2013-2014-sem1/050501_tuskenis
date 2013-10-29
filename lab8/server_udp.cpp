#include "server.h"

namespace 
{
    #define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];

    struct file_info {
        FILE *fd;
        int file_size;
        char file_name[BUFFER_SIZE];
        Semaphore sem;
    };

    map<long, struct file_info*> addr_map;

    vector<pid_t> pid_list;
}

void Checkout(long in_addr);

extern int quit_flag;

void UdpServer(int server_socket)
{
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(sockaddr_in);    

    while(quit_flag == 0) {

        for (int i = 0; i < pid_list.size(); i++) {
            waitpid(pid_list[i], NULL, WNOHANG);
        }

        int bytes_read = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&address, &addr_len);

        if (bytes_read < 1) {
            perror("recvfrom()");
            continue;
        }

        long in_addr = address.sin_addr.s_addr; // network address

        Checkout(in_addr);

        if (addr_map.find(in_addr) == addr_map.end()) {
            struct file_info *fi = new file_info;

            if (is_file_exists(buffer) == 1) {
                fprintf(stderr, "File '%s' is already exists.\n", buffer);
                delete fi;
                continue;
            }

            strcpy(fi->file_name, buffer);

            // Open file
            fi->fd = fopen(buffer, "w+");

            if (fi->fd == NULL) {
                delete fi;
                continue;
            }

            // Recieve file size
            bytes_read = recv(server_socket, buffer, BUFFER_SIZE, 0);

            if (bytes_read < 1) {
                perror("recv()");
                fclose(fi->fd);
                delete fi;
                continue;
            }

            fi->file_size = atoi(buffer);

            addr_map[in_addr] = fi;

        } else {

            struct file_info *fi = addr_map[in_addr];
            pid_t p = fork();

            switch (p) {
                case -1:
                    perror("fork()");
                    break;
                case 0:
                    fi->sem.Wait();

                    fwrite(buffer, bytes_read, sizeof(char), fi->fd);

                    if (fsize(fi->file_name) >= fi->file_size) {
                        fclose(fi->fd);
                    }

                    fi->sem.Reset();
                    exit(0);
                default:
                    pid_list.push_back(p);
            }

            // Send reply
            sendto(server_socket, "x", 1, 0, (struct sockaddr*)&address, addr_len);
        }

    }

    for (int i = 0; i < pid_list.size(); i++) {
        waitpid(pid_list[i], NULL, 0);
    }

    pid_list.clear();
}

void Checkout(long in_addr)
{
    if (addr_map.find(in_addr) == addr_map.end())
        return;

    struct file_info *fi = addr_map[in_addr];

    int f_size = fsize(fi->file_name);

    if (f_size == -1 || f_size >= fi->file_size) {
        delete (struct file_info*)fi;
        addr_map.erase(in_addr);
    }
}


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
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

int oob_flag = 0;
int quit_flag = 0;
int bytes_counter = 0;

int client_socket = -1;
int server_socket = -1;
int remote_socket = -1;


void int_handler(int i)
{
	quit_flag = 1;
}

void urg_handler(int i)
{
	char oob_buf;

	oob_flag = 1;

	if(recv(remote_socket, &oob_buf, sizeof(char), MSG_OOB) == -1)
		perror("recv OOB error");
}

int recv_file(int socket_descriptor)
{
    int bytes_read;
    int file_size;
    char buffer[BUFFER_SIZE];
    FILE *fd;

    // Recieve filename
    bytes_read = recv(socket_descriptor, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 1)
        return -1;
    buffer[bytes_read] = '\0';

    // Create file
    fd = fopen(buffer, "w+");
    if (fd == NULL)
        return -1;

    // Recieve file size
    bytes_read = recv(socket_descriptor, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 1)
        return -1;
    buffer[bytes_read] = '\0';
    file_size = atoi(buffer);

    // Recieve file data
	while(bytes_counter < file_size) {
        bytes_read = recv(socket_descriptor, buffer, BUFFER_SIZE, 0);

		if(oob_flag == 1) {
			printf("recieved %d bytes\n", bytes_counter);
			oob_flag = 0;
			continue;
		}

		if(bytes_read < 1) 
			break;

		bytes_counter += bytes_read;

        fwrite(buffer, sizeof(unsigned char), bytes_read, fd);
    }

	printf("Total bytes recieved: %d\n", bytes_counter);

    fclose(fd);
    return 0;
}

int send_file(char *filepath, int socket_descriptor)
{
	char oob_buf = 'z';
    char buffer[BUFFER_SIZE];
    FILE *fd;
    int file_size = fsize(filepath);
    int bytes_read;
	int bytes_sent;

    fd = fopen(filepath, "r");

    if (fd == NULL || file_size < 1)
        return -1;

	itoa(file_size, buffer);

	char *filename = parse_filename(filepath);
    send(socket_descriptor, filename, strlen(filename), 0);	// Send file name
	free(filename);
    send(socket_descriptor, buffer, strlen(buffer), 0); 	// Send file size

    // Send file data
    while (1) {
        bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, fd);

        if (bytes_read <= 0)
            break;

		bytes_sent = send(socket_descriptor, buffer, bytes_read, 0);

		if(send(socket_descriptor, &oob_buf, sizeof(char), MSG_OOB) == -1) {
			perror("send OOB error");
		}

        if(bytes_sent == -1) {
			fclose(fd);
			return -1;
		}

		bytes_counter += bytes_sent;
		printf("sent %d bytes\n", bytes_counter);

        if (bytes_read < BUFFER_SIZE)
            break;
    }

    fclose(fd);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("\n\tUsage:");
        printf("\n\t   To recv files: %s -r <host> <port>", argv[0]);
        printf("\n\t   To send file : %s <filepath> <host> <port>\n\n", argv[0]);
        return 0;
    }

    char *filepath = argv[1];
    char *host = argv[2];
    char *port = argv[3];

    if (!strcmp(argv[1], "-r")) {
		// =============================
        // SERVER MODE: recieving files
		// =============================
		set_sig_handler(SIGINT, int_handler);
		set_sig_handler(SIGURG, urg_handler);

		server_socket = create_tcp_server(host, atoi(port), 10);

    	if (server_socket == -1) {
        	perror("create_tcp_server() errno");
    	}
		else while (quit_flag == 0) {
        	remote_socket = accept(server_socket, NULL, NULL);

        	if (remote_socket == -1) {
				perror("accept() errno");
            	continue;
			} else {
				if(fcntl(remote_socket, F_SETOWN, getpid()) == -1) 
					perror("fcntl() error");
			}

        	if (recv_file(remote_socket) == -1) {
            	perror("recv_file() errno");
			}

        	close(remote_socket);
			bytes_counter = 0;
    	}

    	close(server_socket);
		// =============================
		// END
		// =============================
	}
    else {
		// =============================
        // CLIENT MODE: sending file
		// =============================
	    client_socket = tcp_connect(host, atoi(port));

    	if (client_socket == -1) {
			perror("tcp_connect() errno");
		} else {
			if (send_file(filepath, client_socket) == -1) {
        		perror("send_file() errno");
			}

			close(client_socket);
    	} 
		// =============================
		// END
		// =============================
	}

    return 0;
}

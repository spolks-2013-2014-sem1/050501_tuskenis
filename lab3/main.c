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

#define BUFFER_SIZE 256

int quit_flag = 0;

void sig_handler(int i)
{
	quit_flag = 1;
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
    for (; file_size > 0; file_size -= BUFFER_SIZE) {
        bytes_read = recv(socket_descriptor, buffer, BUFFER_SIZE, 0);

        if (bytes_read < 1)
            break;

        fwrite(buffer, sizeof(unsigned char), bytes_read, fd);
    }

    fclose(fd);
    return 0;
}

int send_file(char *filepath, int socket_descriptor)
{
    char buffer[BUFFER_SIZE];
    FILE *fd;
    int file_size = fsize(filepath);
    int bytes_read;

    fd = fopen(filepath, "r");

    if (fd == NULL || file_size < 1)
        return -1;

	itoa(file_size, buffer);

	char *filename = (char*)parse_filename(filepath); 		// ? gcc warning if without a cast
    send(socket_descriptor, filename, strlen(filename), 0);	// Send file name
	free(filename);
    send(socket_descriptor, buffer, strlen(buffer), 0); 	// Send file size

    // Send file data
    while (1) {
        bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, fd);

        if (bytes_read <= 0)
            break;

        if(send(socket_descriptor, buffer, bytes_read, 0) == -1) {
			fclose(fd);
			return -1;
		}

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
        printf("\n\t   To recv files: %s [-r] [host] [port]", argv[0]);
        printf("\n\t   To send file : %s <filepath> [host] [port]\n\n", argv[0]);
        return 0;
    }

    char *filepath = argv[1];
    char *host = argv[2];
    char *port = argv[3];

    if (!strcmp(argv[1], "-r"))
	{
		// =============================
        // SERVER MODE: recieving files
		// =============================
		set_sig_handler(SIGINT, sig_handler);

		int server_socket = create_tcp_server(host, atoi(port), 10);

    	if (server_socket == -1) {
        	perror("create_tcp_server() errno");
    	}
		else while (quit_flag == 0) {
        	int accepted_socket = accept(server_socket, NULL, NULL);

        	if (accepted_socket == -1) {
				perror("accept() errno");
            	continue;
			}

        	if (recv_file(accepted_socket) == -1) {
            	perror("recv_file() errno");
			}

        	close(accepted_socket);
    	}

    	close(server_socket);
		// =============================
		// END
		// =============================
	}
    else
	{
		// =============================
        // CLIENT MODE: sending file
		// =============================
	    int socket_descriptor = tcp_connect(host, atoi(port));

    	if (socket_descriptor == -1) {
			perror("tcp_connect() errno");
		}
		else if (send_file(filepath, socket_descriptor) == -1) {
        	perror("send_file() errno");
    	}
		else close(socket_descriptor);
		// =============================
		// END
		// =============================
	}

    return 0;
}

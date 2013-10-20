// Header file for sockets_wrap.c lib
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int create_socket(char *protoname);
int bind_socket(int socket_descriptor, char *ip_address, unsigned short port);
int connect_socket(int socket_descriptor, char *host, unsigned short port);


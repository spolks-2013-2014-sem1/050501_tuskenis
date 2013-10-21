// Header file for tcp_wrap lib
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int tcp_socket();
int create_tcp_server(char *ip_address, unsigned short port, int backlog);
int tcp_connect(char *host, unsigned short port);


#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "tcp_wrap.h"

int tcp_socket()
{
    int socket_descriptor = 0;
    struct protoent *protocol = getprotobyname("tcp");

    if (protocol == NULL)
        return -1;

    socket_descriptor = socket(PF_INET, SOCK_STREAM, protocol->p_proto);

    return socket_descriptor;
}

int tcp_bind(int socket_descriptor, char *ip_address, unsigned short port)
{
    struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (!inet_aton(ip_address, &addr.sin_addr))
        return -1;

    return bind(socket_descriptor, (struct sockaddr *) &addr,
                sizeof(addr));
}

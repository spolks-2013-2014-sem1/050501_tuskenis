#include "sockets_wrap.h"

int create_socket(char *protoname)
{
    int so_value = 1;
    int socket_descriptor = 0;
    struct protoent *protocol = getprotobyname(protoname);

    if (protocol == NULL)
        return -1;

    if (!strcmp(protoname, "udp")) {
        socket_descriptor = socket(PF_INET, SOCK_DGRAM, protocol->p_proto);
    } else {
        socket_descriptor = socket(PF_INET, SOCK_STREAM, protocol->p_proto);
    }

    if (socket_descriptor != -1) {
        if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &so_value, sizeof(so_value)) == -1)
            return -1;
    }

    return socket_descriptor;
}

int set_socket_timeout(int socket_descriptor, int seconds)
{
    struct timeval tv;

    memset(&tv, 0, sizeof(struct timeval));

    tv.tv_sec = seconds;

    return setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
}

int bind_socket(int socket_descriptor, char *ip_address,
                unsigned short port)
{
    if (socket_descriptor == -1)
        return -1;

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (!inet_aton(ip_address, &addr.sin_addr)) {
        close(socket_descriptor);
        return -1;
    }

    if (bind(socket_descriptor, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        close(socket_descriptor);
        return -1;
    }

    return socket_descriptor;
}

int connect_socket(int socket_descriptor, char *host, unsigned short port)
{
    if (socket_descriptor == -1)
        return -1;

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (connect(socket_descriptor, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        close(socket_descriptor);
        return -1;
    }

    return socket_descriptor;
}

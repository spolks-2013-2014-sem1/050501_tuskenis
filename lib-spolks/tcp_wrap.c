#include "tcp_wrap.h"

int tcp_socket()
{
	int so_value = 1;
    int socket_descriptor = 0;
    struct protoent *protocol = getprotobyname("tcp");

    if (protocol == NULL)
        return -1;

    socket_descriptor = socket(PF_INET, SOCK_STREAM, protocol->p_proto);

    if(socket_descriptor != -1)
    {
		if(setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &so_value, sizeof(so_value)) == -1) 
			return -1;
	}

    return socket_descriptor;
}

int create_tcp_server(char *ip_address, unsigned short port, int backlog)
{
	int socket_descriptor = tcp_socket();

	if(socket_descriptor == -1)
		return -1;

    struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (!inet_aton(ip_address, &addr.sin_addr))
	{
		close(socket_descriptor);
        return -1;
	}

    if(bind(socket_descriptor, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		close(socket_descriptor);
		return -1;
	}

    if (listen(socket_descriptor, backlog) == -1) {
        close(socket_descriptor);
        return -1;
    }

	return socket_descriptor;
}

int tcp_connect(char *host, unsigned short port)
{
	int socket_descriptor = tcp_socket();

	if(socket_descriptor == -1)
		return -1;

    struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

	if(connect(socket_descriptor, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		close(socket_descriptor);
		return -1;
	}	

	return socket_descriptor;		
}

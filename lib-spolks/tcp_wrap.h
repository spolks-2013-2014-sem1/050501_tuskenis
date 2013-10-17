// Header file for tcp_wrap lib
int tcp_socket();
int create_tcp_server(char *ip_address, unsigned short port, int backlog);
int tcp_connect(char *host, unsigned short port);

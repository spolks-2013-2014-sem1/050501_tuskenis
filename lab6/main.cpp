#include "server.h"

extern void TcpServer(int server_socket);
extern void UdpServer(int server_socket);

int quit_flag = 0;

void int_handler(int i)
{
    quit_flag = 1;
}

void urg_handler(int i )
{
    // dummy
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("\n\tUsage: %s <host> <port> <tcp|udp>\n\n", argv[0]);
        return 0;
    }

    set_sig_handler(SIGINT, int_handler);
    set_sig_handler(SIGURG, urg_handler);

    char *host = argv[1];
    char *port = argv[2];
    char *protocol = argv[3];

    int server_socket = create_socket(protocol);

    if (server_socket == -1) {
        perror("create_socket() error");
        return 0;
    }

    if (bind_socket(server_socket, host, atoi(port)) == -1) {
        perror("bind_socket() error");
        close(server_socket);
        return 0;
    }

    bool is_tcp = !strcmp(protocol, "tcp");

    if (is_tcp)
        TcpServer(server_socket);
    else
        UdpServer(server_socket);

    close(server_socket);
    return 0;
}


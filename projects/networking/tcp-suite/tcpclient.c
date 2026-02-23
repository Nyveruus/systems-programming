#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./tcpclient <IP> <PORT>\n");
        return 1;
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket");
        return 1;
    }
    struct sockaddr_in server;
    connectf(socket_fd, &server);

}

int connectf(int socket_fd, struct sockaddr_in *server, char **argv) {
    memset(server, 0, sizeof(server));
    server->sin_family = AF_INET;
    server->sin_port = htons(atoi(argv[2]));
    server->sin_addr.s_addr = inet_addr(argv[1]);


}

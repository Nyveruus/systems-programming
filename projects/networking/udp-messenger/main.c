#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define NUMBER 10
#define MESSAGE "Hello world"

int connectf(int socket_fd, char *argv[], struct sockaddr_in *server);
void sendf(int socket_fd);

int main(int argc, char *argv[]) {
        if (argc != 3) {
            printf("Usage: ./main <IP> <PORT>");
            return 1;
        }
        int socket_fd;
        if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Socket");
            return 1;
        }
        struct sockaddr_in server;
        if (connectf(socket_fd, argv, &server) == -1) {
            close(socket_fd);
            return 1;
        }
        // write
        sendf(socket_fd);
        // cleanup
        close(socket_fd);
        return 0;
}

int connectf(int socket_fd, char *argv[], struct sockaddr_in *server) {
    memset(server, 0, sizeof(struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_port = htons(atoi(argv[2]));
    server->sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(socket_fd, (struct sockaddr *)server, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        return -1;
    }
    return 0;
}

void sendf(int socket_fd) {
    int n;
    for (int i = 0; i < NUMBER; i++) {
        if ((n = write(socket_fd, MESSAGE, strlen(MESSAGE))) <= 0) {
            printf("Connection closed");
            return;
        }
    }
}

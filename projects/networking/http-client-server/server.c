#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <poll.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BACKLOG 10

int create(int socket_fd, struct sockaddr_in *server);

int main(void) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_in server;
    if (create(socket_fd, &server) == 1) {
        close(socket_fd);
        return 1;
    }
    for (;;) {
        struct sockaddr_in client;
        socklen_t clientlen = sizeof(client);
        int *client_fd = malloc(sizeof(int));

        if ((*client_fd = accept(socket_fd, (struct sockaddr *)&client, &clientlen)) < 0) {
            perror("accept");
            continue;
        }
    }
}

int create(int socket_fd, struct sockaddr_in *server) {
    memset(server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);
    socklen_t addrlen = sizeof(struct sockaddr)
    if (bind(socket_fd, (struct sockaddr *)server, addrlen) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    printf("Listening on port %i", PORT);
    return 0;
}

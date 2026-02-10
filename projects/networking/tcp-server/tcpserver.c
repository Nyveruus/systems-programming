#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// socket
#define IP "127.0.0.1"
#define PORT 5423

#define BACKLOG 10

int main(void) {

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket error");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = inet_addr(IP);

    bind(socket_fd, (struct sockaddr *)&server, sizeof(server));
    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("Bind");
        goto error;
    }

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("Listen");
        goto error;
    }
    // clean up
error:
    close(socket_fd);
    return 1;

}

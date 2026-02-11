#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// socket
#define IP "127.0.0.1"
#define PORT 5423

#define BACKLOG 1

int main(void) {

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket error");
        return 1;
    }

    struct sockaddr_in server, peer;
    memset(&server, 0, sizeof(sockaddr_in));
    memset(&peer, 0, sizeof(sockaddr_in));
    socklen_t addr_size = sizeof(sockaddr_in);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = inet_addr(IP);

    int client_fd = -1;

    if (bind(socket_fd, (struct sockaddr *)&server, addr_size) == -1) {
        perror("Bind");
        goto cleanup;
    }

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("Listen");
        goto cleanup;
    }
    // select


    // accepts
    if ((client_fd = accept(socket_fd, (struct sockaddr *)&peer, &addr_size)) == -1) {
        perror("Accept");
        goto cleanup;
    }
    // clean up
cleanup:
    close(socket_fd);
    if (client_fd != -1) close(client_fd);
    return 1;

}

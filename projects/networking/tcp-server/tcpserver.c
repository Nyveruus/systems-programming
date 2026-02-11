#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

// socket
#define IP "127.0.0.1"
#define PORT 5423

#define BACKLOG 1
#define TIMEOUT 2000

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

    struct pollfd poll[] = {
        { .fd = socket_fd, .events = POLLIN, .revents = 0 }
    };

    size_t size = sizeof(poll) / sizeof(poll[0]);

    for (;;) {
        // check if return event is POLLIN using &
        // == only works if nothing else is set
        int r = poll(poll, size, TIMEOUT);
        if (r > 0 && (poll[0].revents == POLLIN))
            break;
        else if (r == -1) {
            perror("Poll");
            goto cleanup;
        }

    }
    // accept
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

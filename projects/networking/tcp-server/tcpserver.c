#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define IP "127.0.0.1"
#define PORT 5423

#define BACKLOG 1
#define TIMEOUT 2000

#define BUFFER_SIZE 8192

int main(void) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket error");
        return 1;
    }

    struct sockaddr_in server, peer;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    memset(&server, 0, addr_size);
    memset(&peer, 0, addr_size);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

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

    struct pollfd pollt[] = {
        { .fd = socket_fd, .events = POLLIN, .revents = 0 }
    };

    size_t size = sizeof(pollt) / sizeof(pollt[0]);
    char buffer[BUFFER_SIZE] = "";

    for (;;) {
        // check if return event is POLLIN using &
        // == only works if nothing else is set
        int r = poll(pollt, size, TIMEOUT);
        if (r > 0 && (pollt[0].revents & POLLIN))
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

    for (;;) {
        ssize_t red = read(client_fd, &buffer, sizeof(buffer));
        if (red < 0) {
            perror("Read");
            break;
        }

        fwrite(&buffer, 1, red, stdout);
        fflush(stdout);
    }

    // clean up
cleanup:
    close(socket_fd);
    if (client_fd != -1) close(client_fd);
    return 1;

}

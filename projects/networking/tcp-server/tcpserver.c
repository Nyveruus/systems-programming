#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define BACKLOG 5
#define TIMEOUT 2000

#define BUFFER_SIZE 8192

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./tcpserver <IP> <PORT>");
        return 1;
    }

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
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

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
        int r = poll(pollt, size, TIMEOUT);
        if (r > 0 && (pollt[0].revents & POLLIN)) {
            if ((client_fd = accept(socket_fd, (struct sockaddr *)&peer, &addr_size)) == -1) {
                perror("Accept");
                goto cleanup;
            } else {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(peer.sin_addr.s_addr), client_ip, INET_ADDRSTRLEN);
                printf("[client connected: %s]\n", client_ip);
                for (;;) {
                    ssize_t red = read(client_fd, &buffer, sizeof(buffer));
                    if (red < 0) {
                        perror("Read");
                        break;
                    } else if (red == 0) {
                        printf("[client disconnected]\n");
                        break;
                    }
                    fwrite(&buffer, 1, red, stdout);
                    fflush(stdout);
                }
            }
        } else if (r == -1) {
            perror("Poll");
            goto cleanup;
        }
    }

    // clean up
cleanup:
    close(socket_fd);
    if (client_fd != -1) close(client_fd);
    return 1;

}

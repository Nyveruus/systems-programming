#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define BACKLOG 10
#define TIMEOUT 2000

#define BUFFER_SIZE 8192

#define MAX_CLIENTS 100

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./tcpserver <IP> <PORT>\n");
        return 1;
    }
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket error\n");
        return 1;
    }
    struct sockaddr_in server, peer;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    memset(&server, 0, addr_size);
    memset(&peer, 0, addr_size);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    if (bind(socket_fd, (struct sockaddr *)&server, addr_size) == -1) {
        perror("Bind\n");
        goto cleanup;
    }
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("Listen\n");
        goto cleanup;
    }

    struct pollfd pollt[MAX_CLIENTS + 1];
    pollt[0].fd = socket_fd;
    pollt[0].events = POLLIN;
    pollt[0].revents = 0;

    int nfds = 1;
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        pollt[i].fd = -1;
    }

    char buffer[BUFFER_SIZE] = "";
    for (;;) {
        int r = poll(pollt, nfds, TIMEOUT);
        if (r > 0 && (pollt[0].revents & POLLIN)) {
            int client_fd = accept(socket_fd, (struct sockaddr *)&peer, &addr_size);
            if (client_fd == -1) {
                perror("Accept\n");
                continue;
            } else {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(peer.sin_addr.s_addr), client_ip, INET_ADDRSTRLEN);
                printf("\n[client connected: %s]\n", client_ip);
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (pollt[i].fd == -1) {
                        pollt[i].fd = client_fd;
                        pollt[i].events = POLLIN;
                        if (i >= nfds) nfds = i + 1;
                        break;
                    }
                }
            }
        }
        for (int i = 1; i < nfds; i++) {
            if (pollt[i].fd != -1 && (pollt[i].revents & POLLIN)) {
                ssize_t red = read(pollt[i].fd, buffer, sizeof(buffer));
                if (red < 0) {
                    perror("Read\n");
                    close(pollt[i].fd);
                    pollt[i].fd = -1;
                } else if (red == 0) {
                    printf("\n[client disconnected]\n");
                    close(pollt[i].fd);
                    pollt[i].fd = -1;
                } else {
                    fwrite(buffer, 1, red, stdout);
                    fflush(stdout);
                }
            }
        }
        if (r == -1) {
            perror("Poll\n");
            goto cleanup;
        }
    }

cleanup:
    close(socket_fd);
    for (int i = 1; i < nfds; i++) {
        if (pollt[i].fd != -1) close(pollt[i].fd);
    }
    return 0;
}

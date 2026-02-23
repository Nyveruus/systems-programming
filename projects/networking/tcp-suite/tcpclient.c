#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TIMEOUT 2000
#define BUFFER_SIZE 8192

int connectf(int socket_fd, struct sockaddr_in *server, char **argv);
int readaw(int socket_fd);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./tcpclient <IP> <PORT>\n");
        return 1;
    }

    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        return 1;
    }
    struct sockaddr_in server;
    if (connectf(socket_fd, &server, argv) != 0)
        goto cleanup;
    if (readaw(socket_fd) != 0) {
        goto cleanup;
    }
cleanup:
    close(socket_fd);
    return 0;
}

int connectf(int socket_fd, struct sockaddr_in *server, char **argv) {
    memset(server, 0, sizeof(*server));
    server->sin_family = AF_INET;
    server->sin_port = htons(atoi(argv[2]));
    server->sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(socket_fd, (struct sockaddr *)server, sizeof(struct sockaddr)) == -1) {
        perror("Connect");
        return 1;
    }
    return 0;
}

int readaw(int socket_fd) {
    struct pollfd fds[2] = {
        { .fd = socket_fd, .events = POLLIN, .revents = 0 },
        { .fd = STDIN_FILENO, .events = POLLIN, .revents = 0 }
    };
    int nfds = 2;

    char buffer[BUFFER_SIZE];
    ssize_t red;

    for (;;) {
        int r = poll(fds, nfds, TIMEOUT);

        if (r > 0 && (fds[0].revents & POLLIN)) {
            if ((red = read(socket_fd, buffer, sizeof(buffer))) <= 0) {
                fprintf(stderr, "error");
                return 1;
            }
            write(STDOUT_FILENO, buffer, red);
        }

        if (r > 0 && (fds[1].revents & POLLIN)) {
            if ((red = read(STDIN_FILENO, buffer, sizeof(buffer))) <= 0) {
                fprintf(stderr, "error");
                return 1;
            }
            write(socket_fd, buffer, red);
        }
    }
}

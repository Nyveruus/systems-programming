#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <pthread.h>
#include <signal.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BACKLOG 10
#define BUFFER_SIZE 8192

static int keep_running = 1;

int create(int socket_fd, struct sockaddr_in *server);
void *handler(void *arg);

void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(void) {
    signal(SIGINT, signal_handler);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_in server;
    if (create(socket_fd, &server) == 1) {
        close(socket_fd);
        return 1;
    }
    while (keep_running) {
        struct sockaddr_in client;
        socklen_t clientlen = sizeof(client);
        int client_fd = accept(socket_fd, (struct sockaddr *)&client, &clientlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        pthread_t tid;
        if (pthread_create(&tid, NULL, handler, (void *)(intptr_t)client_fd) != 0) {
            perror("pthread_create");
            close(client_fd);
            continue;
        }
        pthread_detach(tid);
    }
    close(socket_fd);
    return 0;
}

int create(int socket_fd, struct sockaddr_in *server) {
    memset(server, 0, sizeof(struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_port = htons(PORT);
    server->sin_addr.s_addr = inet_addr(IP);
    socklen_t addrlen = sizeof(struct sockaddr);
    if (bind(socket_fd, (struct sockaddr *)server, addrlen) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }
    printf("Listening on port %d\n", PORT);
    return 0;
}

void *handler(void *arg) {
    int client_fd = (int)(intptr_t)arg;
    char buffer[BUFFER_SIZE];

    close(client_fd);
    return NULL;
}

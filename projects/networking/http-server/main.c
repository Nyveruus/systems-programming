#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <regex.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BACKLOG 10
#define BUFFER_SIZE 8192

static volatile sig_atomic_t keep_running = 1;

int create(int socket_fd, struct sockaddr_in *server);
void *handler(void *arg);

void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(void) {
    struct sigaction act = {
        .sa_handler = signal_handler,
        .sa_flags = 0,
    };
    sigaction(SIGINT, &act, NULL);
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
        int *client_fd = malloc(sizeof(int));
        if (!client_fd) {
            perror("malloc");
            break;
        }
        *client_fd = accept(socket_fd, (struct sockaddr *)&client, &clientlen);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            if (errno == EINTR) break;
            continue;
        }
        pthread_t tid;
        if (pthread_create(&tid, NULL, handler, (void *)client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
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
    int client_fd = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE];
    ssize_t received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (received > 0) {
        regex_t comp;
        //capture 0: the full match 2: group (filename)
        regmatch_t matches[2];
        regcomp(&comp, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
        if (regexec(&comp, buffer, 2, matches, 0) == 0) {
            buffer[matches[1].rm_eo] = '\0';
            char *url_file = buffer + matches[1].rm_so;
            char *file = get_filename(url_file);

            char extension[32];
            strcpy(extension, //get extension);

            //buld response
            //send to client
        }
        regfree(&regex);
    }

    free(buffer);
    close(client_fd);
    return NULL;
}

char *get_filename (char *file) {
    ;
}

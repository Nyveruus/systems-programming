#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include <signal.h>

static int keep_running = 1;

void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);

    int socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    //set promiscuous mode on interfaces
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            set_promisc(argv[i]);
        }
    } else {
        for () {
            //if lo, then continue
            if () continue;
        }
    }
}

int set_promisc(char *arg) {
    ;
}

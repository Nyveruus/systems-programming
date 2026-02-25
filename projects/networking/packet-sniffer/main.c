#include <stdio.h>
#include <unistd>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <signal.h>

#define MAX_IF 32

static char *promisc_interfaces[MAX_IF];
static int promisc_count = 0;
static int keep_running = 1;

void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    if (geteuid() != 0) {
        fprintf(stderr, "Must run as root\n");
        return 1;
    }

    signal(SIGINT, signal_handler);

    int socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    //set promiscuous mode on interfaces
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            set_promisc(argv[i], socket_fd);
        }
    } else {
        struct if_nameindex *ifnidxs, *intf;
        ifnidxs = if_nameindex();
        if (ifnidxs == NULL) {
            perror("if_nameindex");
            return 1;
        }
        for (intf = ifnidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
            if (strcmp(intf->if_name, "lo") == 0) continue;
            set_promisc(intf->if_name, socket_fd);
        }
        if_freenameindex(ifnidxs);
    }

    //open .pcap file wb
    //write header to .pcap file
    //while running: recvfrom, function: check if from right interface (filter for default or user arg interfaces) write to .pcap
    //cleanup: remove promisc mode from interfaces, close socket, free any
    for (int i = 0; i < promisc_count; i++) {
        unset_promisc(promisc_interfaces[i], socket_fd);
        free(promisc_interfaces[i]);
    }
    close(socket_fd);
    return 0;
}

void set_promisc(char *arg, int socket_fd) {
    /*
    ioctl is necessary for setting promiscuous flag on interface
    ioctl requires socket fd to allow configuration of interface
    struct ifreq stores setting and is passed to ioctl
    ifnamsiz is max size of if name with null terminator
    */

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, arg, IFNAMSIZ - 1);

    if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) < 0) {
        perror("ioctl");
        return;
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(socket_fd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl");
        return;
    }
    promisc_interfaces[promisc_count++] = strdup(arg);
    return;
}

void unset_promisc(char *arg, int socket_fd) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, arg, IFNAMSIZ - 1);

    if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) < 0) {
        perror("ioctl");
        return;
    }
    ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(socket_fd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl");
        return;
    }
    return;
}

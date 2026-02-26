#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <time.h>
#include <sys/time.h>

#define MAX_IF 32
#define TIME_ROTATION 7200

struct __attribute__((packed)) pcap_header {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t reserved1;
    uint32_t reserved2;
    uint32_t snaplen;
    uint32_t link_type;
};

struct __attribute__((packed)) packet_header {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

static char *promisc_interfaces[MAX_IF];
static int promisc_count = 0;
static int keep_running = 1;

void set_promisc(char *arg, int socket_fd);
void unset_promisc(char *arg, int socket_fd);

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
            close(socket_fd);
            return 1;
        }
        for (intf = ifnidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
            if (strcmp(intf->if_name, "lo") == 0) continue;
            set_promisc(intf->if_name, socket_fd);
        }
        if_freenameindex(ifnidxs);
    }
    //open .pcap file wb
    FILE *file = fopen("captures/file.pcap", "wb");
    if (!file) {
        perror("fopen");
        goto cleanup;
    }
    //write header to .pcap file
    struct pcap_header phdr = {
        .magic_number = 0xA1B2C3D4,
        .version_major = 2,
        .version_minor = 4,
        .reserved1 = 0,
        .reserved2 = 0,
        .snaplen = 65535,
        .link_type = 1
    };
    fwrite(&phdr, sizeof(phdr), 1, file);

    //while running: recvfrom, function: check if from right interface (filter for default or user arg interfaces) write to .pcap
    unsigned char buffer[65536];
    time_t start_time = time(NULL);

    while (keep_running) {
        //while running: if time elapsed over X hours, overwrite file (close and open) and rebuild headers
        if (time(NULL) - start_time >= TIME_ROTATION) {
            fclose(file);
            file = fopen("captures/file.pcap", "wb");
            if (!file) {
                perror("fopen");
                goto cleanup;
            }
            fwrite(&phdr, sizeof(phdr), 1, file);
            start_time = time(NULL);
        }

        struct sockaddr_ll src_addr;
        socklen_t addr_len = sizeof(src_addr);
        ssize_t len = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addr_len);
        if (len < 0) {
            if (keep_running)
                perror("recvfrom");
            break;
        } else if (len == 0 || len > 65535) continue;

        char ifname[IFNAMSIZ];
        if_indextoname(src_addr.sll_ifindex, ifname);
        if (argc > 1) {
            int match = 0;
            for (int i = 1; i < argc; i++)
                if (strcmp(ifname, argv[i]) == 0) {
                    match = 1;
                    break;
                }
            if (!match) continue;
        } else {
            if (strcmp(ifname, "lo") == 0) continue;
        }

        struct timeval tv;
        gettimeofday(&tv, NULL);

        struct packet_header pkt_hdr = {
            .ts_sec = (uint32_t)tv.tv_sec,
            .ts_usec = (uint32_t)tv.tv_usec,
            .incl_len = (uint32_t)len,
            .orig_len = (uint32_t)len
        };

        fwrite(&pkt_hdr, sizeof(pkt_hdr), 1, file);
        fwrite(buffer, len, 1, file);
        fflush(file);
    }
    //cleanup: remove promisc mode from interfaces, close socket, free
cleanup:
    for (int i = 0; i < promisc_count; i++) {
        unset_promisc(promisc_interfaces[i], socket_fd);
        free(promisc_interfaces[i]);
    }
    close(socket_fd);
    if (file) fclose(file);
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

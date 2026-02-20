/* https://linux.die.net/man/7/raw
   https://www.binarytides.com/raw-sockets-c-code-linux/
   https://thelinuxcode.com/socket-programming-in-c-building-networked-applications-from-the-ground-up/

   https://www.rfc-editor.org/rfc/rfc793
   https://www.rfc-editor.org/rfc/rfc791

   https://www.geeksforgeeks.org/computer-networks/calculation-of-tcp-checksum/
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h> //for checking EINTR
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define SIZE 4096
#define IP_SIZE 20
#define SOURCE_PORT 45677
#define WINDOW_SIZE 65535

typedef struct {
   char src_ip[IP_SIZE];
   char dst_ip[IP_SIZE];
   int src_port;
   int port_start;
   int port_end;
   struct in_addr dest;
} scan_config;

typedef struct {
   uint32_t source_addr;
   uint32_t dest_addr;
   uint8_t zero;
   uint8_t protocol;
   uint16_t tcp_len;
} pseudo_header;
//pseudo header adapted from RFC 793, 96 bit size

int local_ipget(char *src);

void *listen_synacks(void *arg);
void process(unsigned char *buffer, int length, scan_config *config);

int syn_scan(scan_config *config, int port);
void build_iph(struct iphdr *iph, scan_config *config);
void build_tcph(struct tcphdr *tcph, struct iphdr *iph, scan_config *config, int port);

uint16_t checksum(void *headers, int size);
uint16_t tcp_checksum(struct tcphdr *tcph, struct iphdr *iph);


int main(int argc, char *argv[]) {
   if (argc < 4) {
      fprintf(stderr, "Usage: ./main <IP> <PORT START> <PORT END>\n");
      return 1;
   }
   //set important vars
   char *target = argv[1];
   int start = atoi(argv[2]);
   int end = atoi(argv[3]);
   struct in_addr dest;

   if (inet_addr(target) != INADDR_NONE)
      dest.s_addr = inet_addr(target);
   else {
      fprintf(stderr, "IP invalid\n");
      return 1;
   }

   char src_ip[IP_SIZE];
   if (local_ipget(src_ip) != 0)
      return 1;

   // build config
   scan_config config;
   strncpy(config.src_ip, src_ip, sizeof(config.src_ip));
   strncpy(config.dst_ip, target, sizeof(config.dst_ip));
   config.src_port = SOURCE_PORT;
   config.port_start = start;
   config.port_end = end;
   config.dest = dest;

   fprintf(stdout, "Scanning ports %i-%i %s\n", start, end, target);

   //start thread to listen for synacks
   pthread_t tid;
   if (pthread_create(&tid, NULL, listen_synacks, &config) != 0) {
      perror("pthread");
      return 1;
   }
   pthread_detach(tid);
   //sleep 0.1s
   usleep(100000);

   //send syns (syn scan)
   for (int i = start; i <= end; i++) {
      if (syn_scan(&config, i) != 0)
         fprintf(stderr, "Failed to send syn to port %i\n", i);
      usleep(5000);
   }
   sleep(2);
   fprintf(stdout, "Finished\n");
   return 0;
}

//find local ip (necessary to construct packets), udp socket trick
int local_ipget(char *src) {
   //This works by creating a udp socket and using the getsockname function to find local IP, packets are never sent or read.
   int sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) {
      perror("socket");
      return 1;
   }

   struct sockaddr_in r;
   memset(&r, 0, sizeof(r));
   r.sin_family = AF_INET;
   r.sin_port = htons(80);
   r.sin_addr.s_addr = inet_addr("1.1.1.1");

   if (connect(sock, (struct sockaddr *)&r, sizeof(r)) < 0) {
      perror("connect");
      close(sock);
      return 1;
   }

   struct sockaddr_in loc;
   socklen_t len = sizeof(loc);

   //getsockname returns the address that the socket is bound on
   if (getsockname(sock, (struct sockaddr *)&loc, &len) < 0) {
      perror("getsockname");
      close(sock);
      return 1;
   }
   strncpy(src, inet_ntoa(loc.sin_addr), 19);
   close(sock);
   return 0;
}

//listen for synacks pthread
void *listen_synacks(void *arg) {
   /*
    Because raw sockets are being used, bind, listen, select, accept are unnecessary they are for tcp connections managed by kernel
    With sockraw there is no concept of connection and handshake unless it is implemented manually, so we will just read packets and
    filter for synacks
    */
   scan_config *config = (scan_config *)arg;

   int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
   if (socket_fd < 0) {
      perror("socket");
      return NULL;
   }

   unsigned char buffer[SIZE];

   for (;;) {
      int readv = read(socket_fd, buffer, SIZE);
      if (readv < 0) {
         //EINTR is not a real error, so we don't want to break the loop if that signal arrives
         if (errno == EINTR) continue;
         perror("read");
         break;
      }
      //call func to read and filter for packets
      process(buffer, readv, config);
   }
   close(socket_fd);
   return NULL;
}

//process incoming packets
void process(unsigned char *buffer, int length, scan_config *config) {
   //only TCP, only packets aimed at source , if syn and ack received then port open, else if RST received then closed port
   //sanity check
   if (length < (int)(sizeof(struct iphdr) + sizeof(struct tcphdr)))
      return;

   struct iphdr *iph = (struct iphdr *)buffer;
   if (iph->protocol != IPPROTO_TCP)
      return;
   if (iph->saddr != config->dest.s_addr)
      return;

   int iph_len = iph->ihl * 4;
   struct tcphdr *tcph = (struct tcphdr *)(buffer + iph_len);

   if (ntohs(tcph->dest) != config->src_port)
      return;
   int src_port = ntohs(tcph->source);
   if (src_port < config->port_start || src_port > config->port_end)
      return;

   if (tcph->syn && tcph->ack)
      fprintf(stdout, "Port %i open (syn-ack received)\n", src_port);
   return;
}

//syn scan: send syn to config destination IP and port
int syn_scan(scan_config *config, int port) {
   /*
   setsockopt specifies to the kernel that we supply the IP header
   write requires an associated address, sendto is better because we can specify destination per call
   */
   int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
   if (socket_fd < 0) {
      perror("socket");
      return 1;
   }
   int o = 1;
   if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &o, sizeof(o)) < 0) {
      perror("setsockopt");
      close(socket_fd);
      return 1;
   }

   char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
   memset(packet, 0, sizeof(packet));
   //create the headers inside of the packet through casting and pointer arithmetic
   struct iphdr *iph = (struct iphdr *)packet;
   struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

   //build headers and insert checksums
   build_iph(iph, config);
   build_tcph(tcph, iph, config, port);

   struct sockaddr_in dest;
   memset(&dest, 0, sizeof(dest));
   dest.sin_family = AF_INET;
   dest.sin_port = htons(port);
   dest.sin_addr.s_addr = config->dest.s_addr;

   int sent = sendto(socket_fd, packet, sizeof(packet), 0, (struct sockaddr *)&dest, sizeof(dest));

   close(socket_fd);
   return sent < 0 ? 1 : 0;
}

//ip header
void build_iph(struct iphdr *iph, scan_config *config) {
   iph->ihl = 5;
   iph->version = 4;
   iph->tos = 0;
   iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
   iph->id = htons((uint16_t)rand());
   iph->frag_off = 0;
   iph->ttl = 64;
   iph->protocol = IPPROTO_TCP;
   iph->check = 0; //initialize
   iph->saddr = inet_addr(config->src_ip);
   iph->daddr = config->dest.s_addr;

   iph->check = checksum(iph, sizeof(struct iphdr));
}
//tcp header
void build_tcph(struct tcphdr *tcph, struct iphdr *iph, scan_config *config, int port) {
   tcph->source = htons(config->src_port);
   tcph->dest = htons(port);
   tcph->seq = htonl(rand());
   tcph->ack_seq = 0;
   tcph->doff = 5;
   tcph->syn = 1;
   tcph->window = htons(WINDOW_SIZE); //how much until ack
   tcph->check = 0;
   tcph->urg_ptr = 0;

   tcph->check = tcp_checksum(tcph, iph);
}

//checksum: for iph and tcp (after tcp has been properly processed with pseudo header)
uint16_t checksum(void *data, int len) {
   uint16_t *p = data;
   uint32_t sum = 0;

   for (; len > 1; len -= 2)
      sum += *p++;
   if (len == 1)
      sum += *(uint8_t *)p;

   sum = (sum >> 16) + (sum & 0xffff);
   sum += (sum >> 16);
   return ~sum;
}

//tcp checksum preparation: combining pseudo header and tcp header
uint16_t tcp_checksum(struct tcphdr *tcph, struct iphdr *iph) {
   //fill pseudo header, the ncombine pseudo header and tcp header into one buffer before returning the checksum() of them
   pseudo_header pseudo;
   pseudo.source_addr = iph->saddr;
   pseudo.dest_addr = iph->daddr;
   pseudo.zero = 0;
   pseudo.protocol = IPPROTO_TCP;
   pseudo.tcp_len = htons(sizeof(struct tcphdr));

   char buffer[sizeof(pseudo_header) + sizeof(struct tcphdr)];
   //put pseudo header into buffer and then use poitner arithmetic to put tcph directly after
   memcpy(buffer, &pseudo, sizeof(pseudo_header));
   memcpy(buffer + sizeof(pseudo_header), tcph, sizeof(struct tcphdr));

   return checksum(buffer, sizeof(buffer));
}

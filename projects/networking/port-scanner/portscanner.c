/* https://linux.die.net/man/7/raw
   https://www.binarytides.com/raw-sockets-c-code-linux/
   https://thelinuxcode.com/socket-programming-in-c-building-networked-applications-from-the-ground-up/

   https://www.rfc-editor.org/rfc/rfc793
   https://www.rfc-editor.org/rfc/rfc791
*/

// set up scan config struct (src ip, port, start, end)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SIZE 4096
#define IP_SIZE 20
#define SOURCE_PORT 45677

int local_ipget(char *src);
void *listen_synacks(void *arg);

typedef struct {
   char src_ip[IP_SIZE];
   char dst_ip[IP_SIZE];
   int src_port;
   int port_start;
   int port_end;
   struct in_addr dest;
} scan_config;

//pseudo header

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

   if (inet_addr(target) != -1)
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
   config.dest = dest;

   printf("Scanning ports %i-%i %s", start, end, target);

   //start thread to listen for synacks
   pthread_t tid;
   if (pthread_create(&tid, NULL, listen_synacks, &config) != 0) {
      perror("pthread");
      return 1;
   }
   //send syns (syn scan)

}

//find local ip, udp socket trick
int local_ipget(char *src) {
   int sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) {
      perror("socket");
      return 1;
   }

   struct sockaddr_in r;
   memset(&remote, 0, sizeof(r));
   r.sin_family = AF_INET;
   r.sin_port = htons(80);
   remote.sin_addr.s_addr = inet_addr("1.1.1.1");

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
   scan_config *config = (scan_config *)arg;

   int socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
   if (socket < 0) {
      perror("socket");
      return NULL;
   }

   unsigned buffer[SIZE];
   /*
   Because raw sockets are being used, bind, listen, select, accept are unnecessary they are for tcp connections managed by kernel
   With sockraw there is no concept of connection and handshake unless it is implemented manually, so we will read packets and
   filter for synacks
   */
   for (;;) {
      int readv = read()
      if (readv <= 0) {

      }
      //call func
   }

   close(socket);
   return NULL;
}

//ip header
//tcp header
//process packets
//syn scan
//ip checksum
//tcp checksum


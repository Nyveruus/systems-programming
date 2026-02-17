/* https://linux.die.net/man/7/raw
   https://www.binarytides.com/raw-sockets-c-code-linux/
   https://thelinuxcode.com/socket-programming-in-c-building-networked-applications-from-the-ground-up/

   https://www.rfc-editor.org/rfc/rfc793
   https://www.rfc-editor.org/rfc/rfc791
*/

// set up scan config struct (src ip, port, start, end)

#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SIZE 4096

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

   //get local ip
   char src_ip[20];
   if (local_ipget(src_ip) != 0)
      return 1;

   int src_port = 45677;

   //start thread to listen for synacks
   //send syns (syn scan)

}

//find local ip, udp socket trick
int local_ipget(char *src) {

}

//init scan config?
//ip header
//tcp header
//listen for synacks
//process packets
//syn scan
//ip checksum
//tcp checksum


/* https://linux.die.net/man/7/raw
   https://www.binarytides.com/raw-sockets-c-code-linux/
   Not applicable but relevant: https://tuttlem.github.io/2025/01/23/building-a-packet-sniffer-with-raw-sockets-in-c.
   https://tuttlem.github.io/2025/01/23/building-a-packet-sniffer-with-raw-sockets-in-c.html
   https://thelinuxcode.com/socket-programming-in-c-building-networked-applications-from-the-ground-up/
*/

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


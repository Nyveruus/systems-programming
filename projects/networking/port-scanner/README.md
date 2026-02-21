# The Program

This is a modular SYN (half-open) port scanner with multithreading and raw packet construction. It builds IP and TCP headers from scratch and manually computes checksums for the headers. Raw packet construction is necessary for sending SYN packets and not completing the handshake.

## Architecture

The program begins with argument parsing and argument validation, then retrieving the local IP - which involves using a temporary udp socket (data is never sent) - for the future packet construction. It puts all important vars into a config that later gets passed around through a pointer. 

It creates a separate thread (detached) dedicated to listening for synacks on the source port, it works by creating a raw socket that receives all incoming tcp packets in a loop, reading it to a buffer and calling the processing function for the packet on each iteration. The processing function takes the buffer, size and config as arguments and it creates header variables for ip and tcp by casting the buffer as an iphdr and tcphdr (with pointer arithmetic for the tcphdr) to do a series of return guards to affirm whether the packet is a syn ack coming from the correct IP and port; it checks if the source addr in iph and the destination addr in the config match up, it checks if the ports match up and checks if the syn and ack bit fields are true, if they are, then the port is open.

The main thread is in charge of sending the syns in a for loop and it begins by calling a wrapper function that opens the socket in SOCK_RAW with the option IP_HDRINCL - which is what allows us to construct the packet ourselves - and creates the packet in memory, defines where the headers are in the packet through casting and pointer arithmetic before calling other build functions to construct the headers by reference which also call the checksum functions while building. The tcp header checksum is computed from the pseudo header in addition to the tcp header itself. It should be noted that for the IP header, the IP checksum and total length will always be computed and overwritten by the kernel, therefore it would be redundant to fill these two fields out yourself. Once the packet is fully constructed, the wrapper function sends it.

Important documentation:
- https://linux.die.net/man/7/raw
- https://www.rfc-editor.org/rfc/rfc793
- https://www.rfc-editor.org/rfc/rfc791

## Usage

```
$ sudo portscanner <ip> <start_port> <end_port>
```

## Installation

```
$ make
$ sudo make install
```

## Example

```
$ sudo portscanner 127.0.0.1 1 150
Scanning ports 1-150 on 127.0.0.1
Port 56 open (syn-ack received)
Port 77 open (syn-ack received)
Finished
```

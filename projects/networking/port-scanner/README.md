# The Program

This is a modular SYN (half-open) port scanner with multithreading and raw packet construction. It builds IP and TCP headers from scratch and manually computes checksums for the headers.

## Architecture



https://thelinuxcode.com/socket-programming-in-c-building-networked-applications-from-the-ground-up/
https://linux.die.net/man/7/raw
https://www.rfc-editor.org/rfc/rfc793
https://www.rfc-editor.org/rfc/rfc791

## Usage

```
$ sudo portscanner <ip> <start_port> <end_port>
```

## Installation

```
$ make
$ sudo make install
```

## Examples

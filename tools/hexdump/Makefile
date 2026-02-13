.PHONY: all install clean uninstall

all: main

main: main.c
	gcc -Wall -Wextra main.c -o hex3
install: hex3
	install -m 755 hex3 /usr/local/bin/hex3
clean:
	rm -f hex3
uninstall:
	rm -f /usr/local/bin/hex3 

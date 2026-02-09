.PHONY: all install clean uninstall

all: hex3

hex3: hex3.c
	gcc -Wall -Wextra hex3.c -o hex3
install: hex3
	install -m 755 hex3 /usr/local/bin/hex3
clean:
	rm -f hex3
uninstall:
	rm -f /usr/local/bin/hex3 

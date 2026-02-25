.PHONY: all install clean uninstall

all: portscanner

portscanner: main.c
	gcc -Wall -Wextra main.c -o portscanner -lpthread
install: portscanner
	install -m 744 portscanner /usr/local/bin/portscanner
clean:
	rm -f portscanner
uninstall:
	rm -f /usr/local/bin/portscanner

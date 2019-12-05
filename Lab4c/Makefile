# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC = gcc
CFLAGS = -Wall -Wextra -g -lm

default:
	echo `uname -a` | grep -e beaglebone -e 304925492\.lasr\.cs\.ucla\.edu;\
	if [ $$? -eq 0 ]; then \
		$(CC) $(CFLAGS) -lmraa -o lab4c_tcp lab4c_tcp.c; \
		$(CC) $(CFLAGS) -lmraa -lssl -lcrypto -o lab4c_tls lab4c_tls.c; \
	else \
		$(CC) $(CFLAGS) -Wno-unused-parameter -DDUMMY -o lab4c_tcp lab4c_tcp.c; \
		$(CC) $(CFLAGS) -Wno-unused-parameter -DDUMMY -lssl -lcrypto -o lab4c_tls lab4c_tls.c; \
	fi;
dist:
	tar -czf lab4c-304925492.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
clean:
	rm -f lab4c_tcp *.tar.gz *.o *~


# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC = gcc
CFLAGS = -Wall -Wextra -g

default: lab3a.c
	$(CC) $(CFLAGS) -o lab3a lab3a.c
dist:
	tar -czf lab3a-304925492.tar.gz lab3a.c Makefile README ext2_fs.h
clean:
	rm -f lab3a *.tar.gz *.o *~

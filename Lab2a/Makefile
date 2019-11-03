# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC=gcc
CFLAGS=-Wall -Wextra -g -pthread

build: lab2_add.c SortedList.c
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c
	$(CC) $(CFLAGS) -o lab2_list SortedList.c SortedList.h lab2_list.c

tests: build
	@-./add_test.sh
	@-./list_test.sh

graphs: build tests
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist: build graphs 
	tar -czf lab2a-304925492.tar.gz Makefile README lab2_add.c SortedList.c lab2_list.c SortedList.h *.csv *.png *.gp add_test.sh list_test.sh

clean:
	rm -f lab2_add lab2_list *~ *.tar.gz

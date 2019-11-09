# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC=gcc
CFLAGS=-Wall -Wextra -g -pthread

build: lab2_list.c SortedList.c
	$(CC) $(CFLAGS) -o lab2_list SortedList.c SortedList.h lab2_list.c

profile: build
	-rm -f ./raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so.0 \
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadfunc ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

tests: build
	./tests.sh

graphs: tests
	./graph2b.gp

dist: profile graphs 
	tar -czf lab2b-304925492.tar.gz Makefile README SortedList.c lab2_list.c SortedList.h *.csv *.png *.gp tests.sh profile.out

clean:
	rm -f lab2_list *~ *.tar.gz

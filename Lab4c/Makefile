# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC = gcc
CFLAGS = -Wall -Wextra -g -lm

default:
	echo `uname -a` | grep -e beaglebone -e 304925492\.lasr\.cs\.ucla\.edu;\
	if [ $$? -eq 0 ]; then \
		$(CC) $(CFLAGS) -lmraa -o lab4c_tcp lab4c_tcp.c; \
	else \
		$(CC) $(CFLAGS) -Wno-unused-parameter -DDUMMY -o lab4c_tcp lab4c_tcp.c; \
	fi;
check: default
	errors=0; \
	{ echo "START"; sleep 2; echo "SCALE=F"; sleep 1; echo "PERIOD=2"; sleep 1; echo "STOP"; sleep 1; echo "OFF"; } | ./lab4c_tcp --log=test.txt --scale=C; \
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi;\
	grep START test.txt;\
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi;\
	grep SCALE=F test.txt ;\
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi;\
	grep PERIOD=2 test.txt;\
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi;\
	grep STOP test.txt;\
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi;\
	grep OFF test.txt; \
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi;\
	egrep [0-9]*:[0-9]*:[0-9]* test.txt;\
	if [ $$? -ne 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi; \
	./lab4b --bogus; \
	if [ $$? -eq 0 ]; then \
		echo "FAIL"; \
		errors=$$errors+1 ;\
	fi; \
	if [ $$errors -eq 0 ]; then \
		echo "PASSES MAKE CHECK";\
	fi;
dist:
	tar -czf lab4c-304925492.tar.gz lab4c_tcp.c Makefile README
clean:
	rm -f lab4c_tcp *.tar.gz *.o *~


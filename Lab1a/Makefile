# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC=gcc
CFLAGS=-Wall -Wextra -g

default: lab1a.c
	$(CC) $(CFLAGS) -o simpsh lab1a.c
clean:
	rm -f simpsh *~ *.tar.gz *.txt
dist:
	tar -czf lab1-304925492.tar.gz lab1a.c Makefile README
check: default testcases

testcases:
	echo "TEST FILE" > testin.txt; \

	./simpsh --rdonly testin.txt
	if [ $$? -eq 0 ]; then \
		echo "Test Case 1 Passes (--rdonly)"; \
	else \
		echo "Test Case 1 FAILS (--rdonly)"; \
	fi; \

	touch test2.txt; \
	./simpsh --wronly test2.txt; \
	if [ $$? -eq 0 ]; then \
		echo "Test Case 2 Passes (--wronly)"; \
	else \
		echo "Test Case 2 FAILS (--wronly)"; \
	fi; \

	touch test3.txt; \
	./simpsh --rdonly testin.txt --wronly test2.txt --wronly test3.txt --command 0 1 2 cat; \
	if [ $$? -eq 0 ]; then\
		echo "Test Case 3 Passes (--command)"; \
	else \
		echo "Test Case 3 FAILS (--command)"; \
	fi; \

	rm -f test*.txt \

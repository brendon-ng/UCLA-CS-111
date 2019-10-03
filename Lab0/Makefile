# NAME: Brendon Ng
# EMAIL: brendonn8@gmail.com
# ID: 304925492

CC=gcc
CFLAGS=-Wall -Wextra -g

default: lab0.c
	$(CC) $(CFLAGS) -o lab0 lab0.c
check: default smoke
clean:
	rm -f lab0.o lab0 *~ *.tar.gz
dist:
	tar -czf lab0-304925492.tar.gz lab0.c Makefile README backtrace.png breakpoint.png

smoke:
	echo "Checking Normal Operation w/ no Arguments"; \
	echo "Test 0" | ./lab0; \
	if [ $$? -eq 0 ]; then \
		echo "Normal Operation w/ no Args Success"; \
	else \
		echo "Normal Operation w/ no Args FAIL"; \
	fi; \

	echo "Checking Normal Operation w/ --input"; \
	echo "Test 1" > input.txt; \
	./lab0 --input=input.txt; \
	if [ $$? -eq 0 ]; then \
                echo "Normal Operation w/ --input Success"; \
        else \
                echo "Normal Operation w/ --input FAIL"; \
        fi; \
	rm -f input.txt; \

	echo "Checking Normal Operation w/ --output"; \
	echo "Test 2" | ./lab0 --output=output.txt; \
	cat output.txt; \
        if [ $$? -eq 0 ]; then \
                echo "Normal Operation w/ --output Success"; \
        else \
                echo "Normal Operation w/ --output FAIL"; \
        fi; \
        rm -f output.txt; \

	echo "Checking Operation w/ --segfault"; \
        echo "Test 3" | ./lab0 --segfault; \
        if [ $$? -eq 139 ]; then \
                echo "Operation w/ --segfault Success"; \
        else \
                echo "Operation w/ --segfault FAIL"; \
        fi; \

	echo "Checking Operation w/ --segfault --catch"; \
        echo "Test 4" | ./lab0 --segfault --catch; \
        if [ $$? -eq 4 ]; then \
                echo "Operation w/ --segfault --catch Success"; \
        else \
                echo "Operation w/ --segfault --catch FAIL"; \
        fi; \

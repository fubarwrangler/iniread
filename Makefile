CC=gcc
CFLAGS=-Wall -O2

%.o: %.c
	$(CC) $(CFLAGS) -c $?

test: test.c iniread.o
	$(CC)

clean:
	rm -f rsort interp

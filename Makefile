CC=gcc
CFLAGS=-Wall -O2
LDFLAGS=

all: test

test: iniread.o initest.o
	$(CC) $(LDFLAGS) -o test $?

%.o: %.c
	$(CC) $(CFLAGS) -c $?

clean:
	rm -f *.o test

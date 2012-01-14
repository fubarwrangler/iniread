CC=gcc
CFLAGS=-Wall -g -O1

test: iniread.c
	$(CC) $(CFLAGS) -D INITESTS -o test $^

clean:
	rm -f *.o test

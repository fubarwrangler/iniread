CC=gcc
CFLAGS=-Wall -g -O2

test: iniread.c initest.c
	$(CC) $(CFLAGS) -o test $^

clean:
	rm -f *.o test

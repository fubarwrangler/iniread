CC=gcc
CFLAGS=-Wall -g -O

test: iniread.c initest.c
	$(CC) $(CFLAGS) -o test $^

clean:
	rm -f *.o test

cc=gcc
CFLAGS=-c -g -Wall -Wextra
LFLAGS=-Wall -Wextra -pthread

.PHONY: all clean

all: multi-lookup

multi-lookup: multi-lookup.o util.o queue.o
	$(CC) $(LFLAGS) $^ -o $@

multi-lookup.o: multi-lookup.c multi-lookup.h
	$(CC) $(CFLAGS) $<

util.o: util.c util.h
	$(CC) $(CFLAGS) $<

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) $<
clean:
	rm -f multi-lookup
	rm -f *.o
	rm -f *~
	rm -f results.txt
	rm -f requester_log.txt
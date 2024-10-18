CC=gcc
CFLAGS=-I.

zombie: zombie.o
	$(CC) -o zombie zombie.o $(CFLAGS)

process_memory: process_memory.o
	$(CC) -o process_memory process_memory.o $(CFLAGS)

parallel_sum: parallel_sum.o libsum.a utils.o
	$(CC) -o parallel_sum parallel_sum.o utils.o -L. -lsum -lpthread $(CFLAGS)

zombie.o: zombie.c
	$(CC) -o zombie.o -c zombie.c $(CFLAGS)

process_memory.o: process_memory.c
	$(CC) -o process_memory.o -c process_memory.c $(CFLAGS)

parallel_sum.o: parallel_sum.c
	$(CC) -o parallel_sum.o -c parallel_sum.c $(CFLAGS)

utils.o: utils.c
	$(CC) -o utils.o -c utils.c $(CFLAGS)

libsum.o: libsum.c
	$(CC) -o libsum.o -c libsum.c $(CFLAGS)

libsum.a: libsum.o
	ar rcs libsum.a libsum.o

clean:
	rm -f *.o process_memory zombie.o zombie parallel_sum libsum.a

all: process_memory zombie parallel_sum

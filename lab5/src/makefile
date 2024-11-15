CC = gcc
CFLAGS = -pthread

all: $(TARGET)

mutex: mutex.o
	$(CC) -o mutex mutex.o $(CFLAGS)

mutex.o: mutex.c
	$(CC) -c mutex.c $(CFLAGS)

factorial: factorial.o
	$(CC) -o factorial factorial.o $(CFLAGS)

factorial.o: factorial.c
	$(CC) -c factorial.c $(CFLAGS)

deadlock: deadlock.o
	$(CC) -o deadlock deadlock.o $(CFLAGS)

deadlock.o: deadlock.c
	$(CC) -c deadlock.c $(CFLAGS)

clean:
	rm -f *.o mutex factorial deadlock

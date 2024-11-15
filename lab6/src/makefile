CC = gcc
CFLAGS = -pthread

server: server.o
	$(CC) -o server server.o $(CFLAGS)

server.o: server.c
	$(CC) -c server.c $(CFLAGS)

client: client.o
	$(CC) -o client client.o $(CFLAGS)

client.o: client.c
	$(CC) -c client.c $(CFLAGS)


clean:
	rm -f *.o client server

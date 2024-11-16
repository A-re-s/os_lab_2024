CC = gcc
CFLAGS = 

tcpclient: tcpclient.o
	$(CC) -o tcpclient tcpclient.o $(CFLAGS)

tcpclient.o: tcpclient.c
	$(CC) -c tcpclient.c $(CFLAGS)

tcpserver: tcpserver.o
	$(CC) -o tcpserver tcpserver.o $(CFLAGS)

tcpserver.o: tcpserver.c
	$(CC) -c tcpserver.c $(CFLAGS)

udpclient: udpclient.o
	$(CC) -o udpclient udpclient.o $(CFLAGS)

udpclient.o: udpclient.c
	$(CC) -c udpclient.c $(CFLAGS)

udpserver: udpserver.o
	$(CC) -o udpserver udpserver.o $(CFLAGS)

udpserver.o: udpserver.c
	$(CC) -c udpserver.c $(CFLAGS)

clean:
	rm -f *.o tcpserver tcpclient udpserver udpclient


all: tcpclient udpclient udpserver tcpserver
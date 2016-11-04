CC=g++
CFLAGS=-Wall

all: udpserver udpclient

udpserver: udpserver.cpp
	$(CC) $(CFLAGS) udpserver.cpp -o udpserver

udpclient: udpclient.cpp
	$(CC) $(CFLAGS) udpclient.cpp -o udpclient

clean:
	rm *.o *~ udpserver udpclient

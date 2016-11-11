# Luke Garrison, Nick Ward
# netid: lgarriso, nward3
# makefile

all: myfrmd

myfrmd: myfrmd.cpp Board.o Message.o
	g++ myfrmd.cpp Board.o Message.o -Wall -std=c++11 -o myfrmd

Board.o: Board.cpp Board.h Message.o
	g++ -c Board.cpp -std=c++11 -o Board.o

Message.o: Message.cpp Message.h
	g++ -c Message.cpp -std=c++11 -o Message.o

clean:
	rm myfrmd *.o

/*
 * Luke Garrison, Nick Ward
 * lgarriso, nward3
 *
 * server side for message board forum
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sstream>
#include <string>
#include <unordered_map>
using namespace std;

#define MAX_MESSAGE_LENGTH 4096

int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg);
int sendMessageTCP(int socketDescriptor, string msg);
int recvMessageUDP(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);
int recvMessageTCP(int socketDescriptor, char* buf, int bufsize);


int main(int argc, char* argv[]) {

	// hash map for usernames and passwords
	unordered_map<string, string> users;

	// check for proper function invocation
	if (argc != 3) {
		cout << "usage: ./myfrmd <port> <password>" << endl;
		exit(1);
	}

	// determine port number
	int port = atoi(argv[1]);
	if (port <= 0) {
		cout << "error: invalid port number" << endl;
		exit(1);
	}

	// determine admin password
	string adminPassword = argv[2];

	// build address data structure
	struct sockaddr_in sin;
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	// udp socket creation
	int socketDescriptorUDP;
	if ((socketDescriptorUDP = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		cout << "error: unable to create udp socket" << endl;
		exit(1);
	}

	// tcp socket creation
	socklen_t len;
	int socketDescriptorTCP;
	if ((socketDescriptorTCP = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "error: unable to create tcp socket" << endl;
		exit(1);
	}

	// set socket option to allow for reuse
	int opt = 1;
	if ((setsockopt(socketDescriptorTCP, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))) < 0) {
		cout << "error: unable to set up socket for reuse" << endl;
		exit(1);
	}

	// bind the socket to the specified address
	if (bind(socketDescriptorTCP, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		cout << "error: unable to bind socket" << endl;
		exit(1);
	}

	// open the passive socket by listening on the socket
	if (listen(socketDescriptorTCP, 1)) {
		cout << "error: unable to listen on socket" << endl;
		exit(1);
	}

	// data transfer buffer
	char buf[MAX_MESSAGE_LENGTH];
	int bufsize = sizeof(buf);
	string command;

	// wait for connection
	bool run = true;
	while(run) {
		
		// try to accept client connection on tcp socket
		int descriptor;
		if ((descriptor = accept(socketDescriptorTCP, (struct sockaddr *)&sin, &len)) < 0) {
			cout << "error: unable to accept client connection" << endl;
			exit(1);
		}

		// send request for username
		sendMessageUDP(socketDescriptorUDP, &sin, "username");

		// checks if new user or existing user and requests password
		recvMessageUDP(socketDescriptorUDP, buf, bufsize, &sin);
		

		// register new user or checks to see if the password matches
		
		
		// send ACK on successful login
		sendMessageUDP(socketDescriptorUDP, &sin, "successfully logged in");
		
		// receive and process client operations
		while(1) {
			if (command == "XIT") {
				close(socketDescriptorUDP);
				close(socketDescriptorTCP);
				break;
			}
		}
	}

	return 0;
}

/* send data udp style */
int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg) {
	int bytesSent;
	int totalBytesSent = 0;
	const char* buf = msg.c_str();
	int msglen = msg.length();

	while (totalBytesSent < msglen) {
		bytesSent = sendto(socketDescriptor, buf + bytesSent, msglen - bytesSent, 0, (struct sockaddr*) &sin, sizeof(struct sockaddr));

		if (bytesSent == -1) {
			perror("server failed to send to client");
			exit(1);
		}

		totalBytesSent += bytesSent;
	}

	return totalBytesSent;
}

/* send data tcp style */
int sendMessageTCP(int socketDescriptor, string msg) {
	int bytesSent;
	int totalBytesSent = 0;
	const char* buf = msg.c_str();
	int msglen = msg.length();

	while (totalBytesSent < msglen) {
		bytesSent = send(socketDescriptor, buf + totalBytesSent, msglen - totalBytesSent, 0);

		if (bytesSent == -1) {
			perror("server failed to send to client");
			exit(1);
		}

		totalBytesSent += bytesSent;
	}

	return totalBytesSent;
}

/* receive data udp style */
int recvMessageUDP(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin) {
	bzero(buf, bufsize);
	socklen_t addr_len;
	
	int recvResult = recvfrom(socketDescriptor, buf, bufsize, 0, (struct sockaddr*)&sin, &addr_len);
	if (recvResult == -1) {
		perror("server failed to receive from client");
		exit(1);
	}

	return recvResult;
}

/* receive data tcp style */
int recvMessageTCP(int socketDescriptor, char* buf, int bufsize) {
	bzero(buf, bufsize);

	int recvResult = recv(socketDescriptor, buf, bufsize, 0);
	if (recvResult == -1) {
		perror("server failed to receive from client");
		exit(1);
	}

	return recvResult;
}

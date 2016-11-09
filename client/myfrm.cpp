/*
 * Luke Garrison, Nick Ward
 * lgarriso, nward3
 *
 * client application for message board forum
 */

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
using namespace std;

#define MAX_MESSAGE_LENGTH 4096

int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg);
int sendMessageTCP(int socketDescriptor, string msg);

int main(int argc, char* argv[]) {
	string hostname;
	int port;
	string textParameter;
	string filename;
	string textData;
	stringstream ss;

	if (argc != 3) {
		cout << "usage: ./myfrm <host name> <port number>" << endl;

		exit(1);
	}

	// use string stream to convert port arg to an integer
	ss.str(argv[2]);
	ss >> port;

	if (ss.fail()) {
		cout << "The port number must be an integer" << endl;
		exit(1);
	}

	/* create TCP connection and UDP socket */

	hostname = argv[1];
	struct hostent* hp = gethostbyname(hostname.c_str());

	if (!hp) {
		cout << "gethostbyname failed" << endl;
		exit(1);
	}

	// build address data structure
	struct sockaddr_in sin;
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy((char*)hp->h_addr, (char*)&sin.sin_addr.s_addr, hp->h_length);
	sin.sin_port=htons(port);

	// create the socket
	int socketDescriptorTCP;
	if ((socketDescriptorTCP = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	// connect to server
	if (connect(socketDescriptorTCP, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		perror("Connection to server failed");
		close(socketDescriptorTCP);
		exit(1);
	}


	// UDP socket creation
	int socketDescriptorUDP;
	if((socketDescriptorUDP = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	char buf[MAX_MESSAGE_LENGTH];
	int bufsize = sizeof(buf);

	// main loop that interprets commands
	string command;
	while(1) {
		cin >> command;

		sendMessageUDP(socketDescriptorUDP, &sin, command);

		if (command == "XIT") {
			close(socketDescriptorTCP);
			close(socketDescriptorUDP);
			exit(0);
		}

		cout << endl;
	}

	free(buf);
	return 0;
}

/* helper function to handle sending a message via udp. handles errors
 */
int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg) {
	int bytesSent;
	int totalBytesSent = 0;
	const char* buf = msg.c_str();
	int msglen = msg.length();

	while (totalBytesSent < msglen) {
		bytesSent = sendto(socketDescriptor, buf + totalBytesSent, msglen - totalBytesSent, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr));

		if (bytesSent == -1) {
			perror("client failed to send to server");
			exit(1);
		}

		totalBytesSent += bytesSent;
	}

	return totalBytesSent;
}

/* helper function to handle sending a message to a server via tcp. handles errors
 */
int sendMessageTCP(int socketDescriptor, string msg) {
	int bytesSent;
	int totalBytesSent = 0;
	const char* buf = msg.c_str();
	int msglen = msg.length();

	while (totalBytesSent < msglen) {
		bytesSent = send(socketDescriptor, buf + totalBytesSent, msglen - totalBytesSent, 0);

		if (bytesSent == -1) {
			perror("client failed to send to server");
			exit(1);
		}

		totalBytesSent += bytesSent;
	}

	return totalBytesSent;
}

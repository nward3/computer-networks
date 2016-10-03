/*
 * Luke Garrison
 * netid: lgarriso
 * myftp.cpp
 *
 * This program is the client for an FTP application. It will connect to specified server and enables different FTP commands.
 *
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
using namespace std;

#define MAX_MESSAGE_LENGTH 5000

int main(int argc, char* argv[]) {
	string hostname;
	int port;
	string textParameter;
	string filename;
	string textData;
	stringstream ss;

	if (argc != 3) {
		cout << "Please provide the following command line arguments" << endl;
		cout << "\t ./myftp <host name> <port number>" << endl;

		exit(1);
	}

	// use string stream to convert port arg to an integer
	ss.str(argv[2]);
	ss >> port;

	if (ss.fail()) {
		cout << "The port number must be an integer" << endl;
		exit(1);
	}

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
	int s;
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	// connect to server
	if (connect(s, (struct sockaddr	*)&sin, sizeof(sin)) < 0) {
		perror("Connection to server failed");
		close(s);
		exit(1);
	}


	// main loop goes here
	string command;
	while(1) {
		cin >> command;

		int sendResult = send(s, command.c_str(), command.length(), 0);
		if (sendResult == -1) {
			perror("client failed to send to server");
			exit(1);
		}
	}

	return 0;
}

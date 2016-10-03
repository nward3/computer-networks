/*
 * Luke Garrison
 * netid: lgarriso
 * udpclient.cpp
 *
 * This program allows the user to connect to a server and send the server data either from a file or text from a command line argument. The server then returns two messages that this program will receieve: the encrypted message with an appended timestamp, and a key to decrypt the message. The encryption key is used to decrypt the message and the decrypted message and the round trip time are then displayed to the user. This client connects to the server via a UDP connection.
 *
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
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
	bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
	sin.sin_port=htons(port);

	// create the socket
	int s;
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	return 0;
}

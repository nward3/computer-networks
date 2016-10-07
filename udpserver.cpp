/* Program 2 -- UDP Server
 name: Nick Ward
 netid: nward3 */

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sstream>
#include <string>
using namespace std;

/* function to encrypt the message from the client */
string encrypt(string message, string key) {
	int keyLength = key.length();
	string response = "";
	for (unsigned int i = 0; i < message.length(); i++) {
		response += message[i] ^ key[i % keyLength];
	}
	return response;
}

int main(int argc, char* argv[]) {

	// define structures to be used
	struct sockaddr_in sin, client_addr;
	char buf[4096];
	int s, len;

	// check for proper number of args
	if (argc != 3) {
		cout << "error: improper function invocation" << endl << "usage: udpserver <portNumber> <encryptionKey>" << endl;
		exit(1);
	}

	// create variables for command line arguments
	int portNumber = atoi(argv[1]);
	string encryptionKey = argv[2];

	// check if conversion of portNumber failed
	if (portNumber == 0) {
		cout << "error: invalid port number" << endl;
		exit(1);
	}	

	// build address data structure
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY; // use the default IP address of server
	sin.sin_port = htons(portNumber);

	// setup passive open; create a socket on the server side
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		cout << "error: socket creation failed" << endl;
		exit(1);
	}

	// bind the created socket to the specified address
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		cout << "error: failed to bind socket to address" << endl;
		exit(1);
	}

	// continuously listen for a client
	while(1) {

		// receive the message and print it to stdout
		string clientMessage = "";
		socklen_t addr_len;
		int messageSize = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
		if (messageSize == -1) {
			cout << "error: unable to receive message from client" << endl;
			exit(1);
		}

		// copy buffer into string
		for (int i = 0; i < messageSize; i++) {
			clientMessage += buf[i];
		}

		// create and append timestamp
		clientMessage += " Timestamp: ";
		time_t t;
		time(&t);
		struct tm *timeinfo = localtime(&t);
		struct timeval seconds;
		gettimeofday(&seconds, NULL);
		stringstream ss;
		ss << timeinfo->tm_hour;
		clientMessage += ss.str();
		ss.str("");
		clientMessage += ":";
		ss << timeinfo->tm_min;
		clientMessage += ss.str();
		ss.str("");
		clientMessage += ":";
		ss << timeinfo->tm_sec;
		clientMessage += ss.str();
		ss.str("");
		clientMessage += ".";
		ss << seconds.tv_usec;
		clientMessage += ss.str();

		// encrypt the message
		string encryptedMessage = encrypt(clientMessage, encryptionKey);

		// return the encrypted message and key to the client
		len = encryptedMessage.length();
		if (sendto(s, encryptedMessage.c_str(), len, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) == -1) {
			cout << "error: unable to return message to client" << endl;
			exit(1);
		}

		len = encryptionKey.length();
		if (sendto(s, encryptionKey.c_str(), len, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) == -1) {
			cout << "error: unable to return message decryption key to client" << endl;
			exit(1);
		}

		// clear buffer and continue to receive the next message
		bzero((char *)&buf, sizeof(buf));
	}
	close(s);

	return 0;
}

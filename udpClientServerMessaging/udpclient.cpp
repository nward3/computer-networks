/* Program 1 -- UDP Client
 Name: Nick Ward
 netid: nward3 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <ctime>
using namespace std;

/* function to decrypt the message from the server */
string decrypt(string message, string key) {
	int keyLength = key.length();
	string response = "";
	for (unsigned int i = 0; i < message.length(); i++) {
		response += message[i] ^ key[i % keyLength];
	}

	return response;
}

int main(int argc, char* argv[]) {

	// define structures to be used
	struct hostent *hp;
	struct sockaddr_in sin, client_addr;
	char buf[4096];
	int s, len;

	// check for proper number of args
	if (argc != 4) {
		cout << "error: improper function invocation" << endl << "usage: udpclient <hostName> <portNumber> <textToSend>" << endl;
		exit(1);
	}

	// create variables for command line arguments
	char *hostName = argv[1];
	int portNumber = atoi(argv[2]);
	char *textToSend = argv[3];

	// check if conversion of portNumber failed
	if (portNumber == 0) {
		cout << "error: invalid port number" << endl;
		exit(1);
	}

	// translate host name into IP address
	hp = gethostbyname(hostName);
	
	// error translating host name
	if (!hp) {
		cout << "error: invalid host name" << endl;
		exit(1);
	}

	// build address data structure
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char	*)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(portNumber);

	// active open
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		cout << "error: unable to create socket" << endl;
		exit(1);
	}
	
	// check if textToSend is a file
	int fd = access(textToSend, R_OK);

	// not a file so treat it as string
	if (fd != 0) {
		strncpy(buf, textToSend, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
	} else {
		FILE *fp;

		// open file and read text
		fp = fopen(textToSend, "r");
		if (!fp) {
			cout << "error: unable to read file's text" << endl;
			exit(1);
		}
		fread(buf, sizeof(buf), 1, fp);
	}

	// send text and get send time
	len = strlen(buf) + 1;
	if(sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		cout << "error: unable to send text" << endl;
		exit(1);
	}

	// determine start time in microseconds
	struct timeval start;
	gettimeofday(&start, NULL);
	long startTime = start.tv_sec * 1000000 + start.tv_usec;

	// initialize variables for end time
	struct timeval end;
	long endTime;

	// wait for server response
	socklen_t addr_len;
	int messages = 0;
	string message = "";
	string key = "";
	while(1) {
		int size = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
		if (size == -1) {
			cout << "error: unable to receive response from server" << endl;
			exit(1);
		}

		messages += 1;

		// encrypted message
		if (messages == 1) {
			for (int i = 0; i < size; i++) {
				message += buf[i];
			}
			gettimeofday(&end, NULL);
			endTime = end.tv_sec * 1000000 + end.tv_usec;
		}
		
		// decryption key
		else if (messages == 2) {
			for (int i = 0; i < size; i++) {
				key += buf[i];
			}
			bzero((char *)&buf, sizeof(buf));
			break;
		}

		bzero((char *)&buf, sizeof(buf));
	}

	// decrypt the server message and determine RTT
	string decrypted = decrypt(message, key);
	long rtt = (endTime - startTime);

	// output the results to the user
	cout << decrypted << endl;
	cout << rtt << endl;

	return 0;
}


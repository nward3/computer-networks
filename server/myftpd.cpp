/* PG 3: Simple File TRansfer Protocol
 * Name: Nick Ward & Luke Garrison
 * netids: nward3 & lgarriso */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;

int main(int argc, char* argv[]) {

	// define data structures to be used
	struct sockaddr_in server_addr;
	char buf[4096];
	socklen_t len;
	int socketfd, bytesReceived, new_s;

	// check for proper function invocation
	if (argc != 2) {
		cout << "usage: myftpd <port>" << endl;
		exit(1);
	}

	// determine port number
	int port = atoi(argv[1]);
	if (port <= 0) {
		cout << "error: invalid port number" << endl;
		exit(1);
	}

	// build address data structure
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	// tcp socket creation
	socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {
		cout << "error: unable to create socket" << endl;
		exit(1);
	}

	// set socket option to allow for reuse
	int opt = 1;
	if ((setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))) < 0) {
		cout << "error: unable set up socket for reuse" << endl;
		exit(1);
	}

	// bind the socket to the specified address
	if (bind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		cout << "error: unable to bind socket" << endl;
		exit(1);
	}

	// open the passive socket by listening on the socket
	if ((listen(socketfd, 1))) {
		cout << "error: unable to listen on socket" << endl;
		exit(1);
	}

	// loop continuously
	while(1) {
		
		// wait for client connection
		if ((new_s = accept(socketfd, (struct sockaddr *) &server_addr, &len)) < 0) {
			cout << "error: unable to accept client connection" << endl;
			exit(1);
		} else {
			cout << "new connection established" << endl;
		}

		// receive client message
		while(1) {
			bytesReceived = recv(new_s, buf, sizeof(buf), 0);
			if (bytesReceived < 0) {
				cout << "error: unable to receive client's message" << endl;
				exit(1);
			} else if (bytesReceived > 0) {
				cout << "TCP server received: " << buf << endl;

				// process client command
				string command = buf;
				bzero(buf, sizeof(buf));
				if (command == "XIT") {
					close(new_s);
					break;
				}
			}	
		}
	}
}

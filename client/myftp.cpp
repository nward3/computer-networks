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
#include <unordered_set>
using namespace std;

#define MAX_MESSAGE_LENGTH 5000

void sendMessage(int socketDescriptor, string message);
string getCommand(string input);
bool isValidCommand(string command);

int main(int argc, char* argv[]) {
	string hostname;
	int port;
	string textParameter;
	string filename;
	string textData;
	stringstream ss;

	if (argc != 3) {
		cout << "usage: ./myftp <host name> <port number>" << endl;

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

	// main loop that interprets commands
	string input, command;
	while(1) {
		getline(cin, input);
		command = getCommand(input);

		if (command == "XIT") {
			sendMessage(s, command);
			close(s);
			exit(0);
		}
	}

	return 0;
}

// sends message to server. Includes error checking
void sendMessage(int socketDescriptor, string message) {
	int sendResult = send(socketDescriptor, message.c_str(), message.length(), 0);
	if (sendResult == -1) {
		perror("client failed to send to server");
		exit(1);
	}
}

// parses input from command line and returns the command
string getCommand(string input) {
	if (input.length() == 0) {
		return input;
	}

	istringstream iss(input);
	string command;
	iss >> command;

	return command;
}

// returns true if command is valid
bool isValidCommand(string command) {
	unordered_set<string> validCommands = {"REQ", "UPL", "DEL", "LIS", "MKD", "RMD", "CHD", "XIT"};

	return validCommands.find(command) != validCommands.end();
}

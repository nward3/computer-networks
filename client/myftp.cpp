/*
 * Luke Garrison, Nick ward
 * netid: lgarriso, nward3
 *
 * This program is the client for an FTP application. It will
 * connect to specified server and enables different FTP commands.
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
#include <dirent.h>
#include <unordered_set>
using namespace std;

#define MAX_MESSAGE_LENGTH 4096

int sendMessage(int socketDescriptor, string message);
int recvMessage(int socketDescriptor, char* buf, int bufsize);
string getCommand(string input);
string getDirCommand(string input);
bool isValidCommand(string command);
void clearBuffer(char* buf, int bufSize);
void makeDirectory(int socketDescriptor, char* buf, int bufsize);

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
	int socketDescriptor;
	if ((socketDescriptor = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	// connect to server
	if (connect(socketDescriptor, (struct sockaddr	*)&sin, sizeof(sin)) < 0) {
		perror("Connection to server failed");
		close(socketDescriptor);
		exit(1);
	}

	char buf[MAX_MESSAGE_LENGTH];
	int bufsize = sizeof(buf);

	// main loop that interprets commands
	string input, command, directoryName;
	while(1) {
		cin >> command;

		if (command == "XIT") {
			sendMessage(socketDescriptor, command);
			close(socketDescriptor);
			exit(0);
		} else if (command == "LIS") {
			sendMessage(socketDescriptor, command);
			recvMessage(socketDescriptor, buf, bufsize);
			cout << endl;
			cout << buf << endl;
		} else if (command == "MKD") {
			sendMessage(socketDescriptor, command);
			makeDirectory(socketDescriptor, buf, bufsize);

		} else {
			sendMessage(socketDescriptor, command);
			recvMessage(socketDescriptor, buf, bufsize);
			cout << buf << endl;
		}

		cout << endl;
	}

	free(buf);
	return 0;
}

// request for the server to create a new directory
void makeDirectory(int socketDescriptor, char* buf, int bufsize) {
	string directoryName;
	cin >> directoryName;

	stringstream ss;
	ss << directoryName.length() << " " << directoryName;
	string msg = ss.str();

	sendMessage(socketDescriptor, msg);
	recvMessage(socketDescriptor, buf, bufsize);

	string statusCode = buf;

	if (statusCode == "-2") {
		cout << "The directory already exists on server" << endl;
	} else if (statusCode == "-1") {
		cout << "Error in making directory" << endl;
	} else {
		cout << "The directory was successfully made" << endl;
	}
}

// clear the buffer
void clearBuffer(char* buf, int bufsize) {
	bzero(buf, bufsize);
}

// sends message to server. Includes error checking
int sendMessage(int socketDescriptor, string message) {
	int sendResult = send(socketDescriptor, message.c_str(), message.length(), 0);
	if (sendResult == -1) {
		perror("client failed to send to server");
		exit(1);
	}

	return sendResult;
}

// sends message to server. Includes error checking
int recvMessage(int socketDescriptor, char* buf, int bufsize) {
	clearBuffer(buf, bufsize);

	int recvResult = recv(socketDescriptor, buf, bufsize, 0);
	if (recvResult == -1) {
		perror("client failed to send receive data from server");
		exit(1);
	}

	return recvResult;
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

// parses input for directory command from command line and returns the command
string getDirCommand(string input) {
	if (input.length() == 0) {
		return input;
	}

	istringstream iss(input);
	string dirSize;
	string dirName;
	string command;
	iss >> dirSize >> dirName;

	command = dirSize + " " + dirName;

	return command;
}

// returns true if command is valid
bool isValidCommand(string command) {
	unordered_set<string> validCommands = {"REQ", "UPL", "DEL", "LIS", "MKD", "RMD", "CHD", "XIT"};

	return validCommands.find(command) != validCommands.end();
}

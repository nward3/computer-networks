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
int recvMessage(int socketDescriptor, char* buf, int buf_size);
string getCommand(string input);
string getDirCommand(string input);
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

	char buf[MAX_MESSAGE_LENGTH];

	// main loop that interprets commands
	string input, command;
	while(1) {
		getline(cin, input);
		command = getCommand(input);

		if (command == "XIT") {
			sendMessage(s, command);
			close(s);
			exit(0);
		} else if (command == "LIS") {
			sendMessage(s, command);
			recvMessage(s, buf, sizeof(buf));
			cout << endl;
			cout << buf << endl;
		} else if (command == "MKD") {
			sendMessage(s, command);

			// build message to send to server
			getline(cin, input);
			command = getDirCommand(input);
			command += " ";
			getline(cin, input);
			command += getDirCommand(input);

			sendMessage(s, command);
			recvMessage(s, buf, sizeof(buf));

			string result = buf;
			
			if (result == "-2") {
				cout << "The directory already exists on server" << endl;
			} else if (result == "-1") {
				cout << "Error in making directory" << endl;
			} else {
				cout << "The directory was successfully made" << endl;
			}
		} else if (command == "RMD") {
			sendMessage(s, command);

			// send the directory name length
			cin >> input;
			sendMessage(s, input);

			// send the directory name
			cin >> input;
			cout << sendMessage(s, input) << endl;
			
			// potential confirmation
			recvMessage(s, buf, sizeof(buf));
			string result = buf;
			bzero(buf, sizeof(buf));
			if (result == "-1") {
				cout << "The directory does not exist on server" << endl;
			} else if (result == "1") {
				cout << "Confirm deletion: Yes/No" << endl;
				getline(cin, input);
				command = getDirCommand(input);
				sendMessage(s, command);
				recvMessage(s, buf, sizeof(buf));
				result = buf;
				if (result == "0") {
					cout << "Directory deleted" << endl;
				} else {
					cout << "Failed to delete directory" << endl;
				}
			}

		} else {
			sendMessage(s, command);
			recvMessage(s, buf, sizeof(buf));
			cout << buf << endl;
		}

		// clear the recv buffer
		bzero(buf, sizeof(buf));

		cout << endl;
	}

	return 0;
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
int recvMessage(int socketDescriptor, char* buf, int buf_size) {
	int recvResult = recv(socketDescriptor, buf, buf_size, 0);
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

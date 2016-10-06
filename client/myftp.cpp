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
string getDirectoryNameAndLength();
void deleteFile(int socketDescriptor, char* buf, int bufsize);
void makeDirectory(int socketDescriptor, char* buf, int bufsize);
void changeDirectory(int socketDescriptor, char* buf, int bufsize);
void removeDirectory(int socketDescriptor, char* buf, int bufsize);
int stringToInt(string s);

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

		sendMessage(socketDescriptor, command);

		if (command == "XIT") {
			close(socketDescriptor);
			exit(0);
		} else if (command == "LIS") {
			recvMessage(socketDescriptor, buf, bufsize);
			cout << endl;
			cout << buf << endl;
		} else if (command == "DEL") {
			deleteFile(socketDescriptor, buf, bufsize);
		} else if (command == "MKD") {
			makeDirectory(socketDescriptor, buf, bufsize);
		} else if (command == "RMD") {
			removeDirectory(socketDescriptor, buf, bufsize);
		} else if (command == "CHD") {
			changeDirectory(socketDescriptor, buf, bufsize);
		} else {
			recvMessage(socketDescriptor, buf, bufsize);
			cout << buf << endl;
		}

		cout << endl;
	}

	free(buf);
	return 0;
}

void deleteFile(int socketDescriptor, char* buf, int bufsize) {
	// send the file name and length to server
	string fileNameAndLength = getDirectoryNameAndLength();
	sendMessage(socketDescriptor, fileNameAndLength);

	// server responds if file exists or not
	recvMessage(socketDescriptor, buf, sizeof(buf));
	int result = stringToInt(buf);
	if (result < 0) {
		cout << "The file does not exist on server" << endl;
	} else {
		// file exists -- confirmation
		
		string confirmation;
		while (confirmation != "Yes" && confirmation != "No") {
			cout << "confirm deletion: Yes/No" << endl;
			cin >> confirmation;
		}

		sendMessage(socketDescriptor, confirmation);
		if (confirmation == "No") {
			cout << "Delete abandoned by the user!" << endl;
			return;
		}

		recvMessage(socketDescriptor, buf, sizeof(buf));
		result = stringToInt(buf);
		if (result > 0) {
			cout <<"File deleted" << endl;
		} else {
			cout << "Failed to delete directory" << endl;
		}
	}
}

void removeDirectory(int socketDescriptor, char* buf, int bufsize) {
	// send the directory name and length to server
	string directoryNameAndLength = getDirectoryNameAndLength();
	sendMessage(socketDescriptor, directoryNameAndLength);
	
	// server responds if directory exists or not
	recvMessage(socketDescriptor, buf, sizeof(buf));
	int result = stringToInt(buf);
	if (result < 0) {
		cout << "The directory does not exist on server" << endl;
	} else {
		// directory exists -- confirmation
	
		string confirmation;
		while (confirmation != "Yes" && confirmation != "No") {
			cout << "Confirm deletion: Yes/No" << endl;
			cin >> confirmation;
		}

		sendMessage(socketDescriptor, confirmation);
		if (confirmation == "No") {
			cout << "Delete abandoned by the user!" << endl;
			return;
		}

		recvMessage(socketDescriptor, buf, sizeof(buf));
		result = stringToInt(buf);
		if (result > 0) {
			cout << "Directory deleted" << endl;
		} else {
			cout << "Failed to delete directory" << endl;
		}
	}
}

/* prompts the user for a directory name and returns the concatenation of 
 * the length of the directory name string and the directory name string
 */
string getDirectoryNameAndLength() {
	string directoryName;
	cin >> directoryName;

	stringstream ss;
	ss << directoryName.length() << " " << directoryName;
	return ss.str();
}

// convert string to integer
int stringToInt(string s) {
	stringstream ss;
	ss << s;

	int i;
	ss >> i;

	return i;
}

// request for the server to change directories
void changeDirectory(int socketDescriptor, char* buf, int bufsize) {
	string directoryNameAndLength = getDirectoryNameAndLength();

	sendMessage(socketDescriptor, directoryNameAndLength);
	recvMessage(socketDescriptor, buf, bufsize);

	string statusCode = buf;

	if (statusCode == "-2") {
		cout << "The specified directory does not exist" << endl;
	} else if (statusCode == "-1") {
		cout << "Unable to change directories" << endl;
	} else {
		cout << "The directory was successfully changed" << endl;
	}
}

// request for the server to create a new directory
void makeDirectory(int socketDescriptor, char* buf, int bufsize) {
	string directoryNameAndLength = getDirectoryNameAndLength();

	sendMessage(socketDescriptor, directoryNameAndLength);
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

// returns true if command is valid
bool isValidCommand(string command) {
	unordered_set<string> validCommands = {"REQ", "UPL", "DEL", "LIS", "MKD", "RMD", "CHD", "XIT"};

	return validCommands.find(command) != validCommands.end();
}

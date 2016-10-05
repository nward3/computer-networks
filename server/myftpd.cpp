/* PG 3: Simple File Transfer Protocol
 * Name: Nick Ward & Luke Garrison
 * netids: nward3 & lgarriso */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <errno.h>
#include <unordered_set>
#include <sstream>
using namespace std;

int sendMessage(int socketDescriptor, string message);
int recvMessage(int socketDescriptor, char* buf, int buf_size);
bool isValidCommand(string command);
bool directoryExists(string directoryName);
string getDirectoryListing(int socketDescriptor);
string createDirectory(string dirInfo);

int main(int argc, char* argv[]) {

	// define data structures to be used
	struct sockaddr_in server_addr;
	char buf[4096];
	socklen_t len;
	int socketfd, bytesReceived, socketDescriptor;

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

	// allow for client to XIT and reconnect
	while(1) {
		
		// wait for client connection
		if ((socketDescriptor = accept(socketfd, (struct sockaddr *) &server_addr, &len)) < 0) {
			cout << "error: unable to accept client connection" << endl;
			exit(1);
		} else {
			cout << "new connection established" << endl;
		}

		// receive and process client message
		while(1) {
			bytesReceived = recvMessage(socketDescriptor, buf, sizeof(buf));

			if (bytesReceived < 0) {
				cout << "error: unable to receive client's message" << endl;
				exit(1);
			} else if (bytesReceived > 0) {
				cout << "TCP server received: " << buf << endl;

				// process client command
				string command = buf;

				// clear the recv buffer
				bzero(buf, sizeof(buf));

				if (command == "XIT") {
					close(socketDescriptor);
					break;
				} else if (command == "LIS") {
					string dirlisting = getDirectoryListing(socketDescriptor);
					sendMessage(socketDescriptor, dirlisting);
				} else if (command == "MKD") {
					bytesReceived = recvMessage(socketDescriptor, buf, sizeof(buf));
					string bufcommand = buf;
					if (bytesReceived < 0) {
						cout << "error: unable to receive client's message" << endl;
						exit(1);
					} else if (bytesReceived > 0) {
						string dirInfo = buf;
						bzero(buf, sizeof(buf));
						string result = createDirectory(dirInfo);
						sendMessage(socketDescriptor, result);
					}
					
				} else {
					sendMessage(socketDescriptor, "Invalid FTP command");
				}
			}	
		}
	}
}

// create directory if it does not exist
string createDirectory(string input) {
	istringstream iss(input);
	int dirNameLength;
	string dirName;

	iss >> dirNameLength >> dirName;

	if (directoryExists(dirName)) {
		return "-2";
	} else {
		// try to create directory
		if (mkdir(dirName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
			return "-1";
		} else {
			return "1";
		}
	}

	return "";
}

// check if directory already exists
bool directoryExists (string pzPath) {

	if (pzPath.length() == 0) return false;

	DIR *pDir;
	bool bExists = false;

	pDir = opendir(pzPath.c_str());

	if (pDir != NULL) {
		bExists = true;    
		closedir(pDir);								
	}
	return bExists;
}

/* returns a string containing the files and directories in the current
 * directory each file or directory is separated by a newline */
string getDirectoryListing(int socketDescriptor) {
	DIR *dir;
	struct dirent *ent;
	string dirlisting;

	if ((dir = opendir (".")) != NULL) {
		/* iterate over all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			dirlisting += (ent->d_name + string("\n"));
		}
		closedir(dir);

		return dirlisting;
	} else {
		/* could not open directory */
		return string(strerror(errno));
	}
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

// returns true if command is valid
bool isValidCommand(string command) {
	unordered_set<string> validCommands = {"REQ", "UPL", "DEL", "LIS", "MKD", "RMD", "CHD", "XIT"};

	return validCommands.find(command) != validCommands.end();
}

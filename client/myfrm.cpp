/*
 * Luke Garrison, Nick Ward
 * lgarriso, nward3
 *
 * client application for message board forum
 */

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
using namespace std;

#define MAX_MESSAGE_LENGTH 4096

int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg);
int sendMessageTCP(int socketDescriptor, string msg, int msglen);
int recvMessageUDP(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);
int recvMessageTCP(int socketDescriptor, char* buf, int bufsize);
void promptUserForOperation();
bool shutdownServer(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in* sinUDP);
void sendAndReceiveBoardRequest(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in* sinUDP);
void addMessageToBoard(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP);
void deleteMessageFromBoard(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP);
void editMessage(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP);
void listBoards(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP);
void appendFileToBoard(int socketDescriptorUDP, int socketDescriptorTCP, char* buf, int bufsize, struct sockaddr_in* sinUDP);
bool fileExists(string filename);

int main(int argc, char* argv[]) {
	string hostname;
	int port;
	string textParameter;
	string filename;
	string textData;
	stringstream ss;

	if (argc != 3) {
		cout << "usage: ./myfrm <host name> <port number>" << endl;

		exit(1);
	}

	// use string stream to convert port arg to an integer
	ss.str(argv[2]);
	ss >> port;

	if (ss.fail()) {
		cout << "The port number must be an integer" << endl;
		exit(1);
	}

	/* create TCP connection and UDP socket */

	hostname = argv[1];
	struct hostent* hp = gethostbyname(hostname.c_str());

	if (!hp) {
		cout << "gethostbyname failed" << endl;
		exit(1);
	}

	// build address data structure
	struct sockaddr_in sinTCP;
	bzero((char*)&sinTCP, sizeof(sinTCP));
	sinTCP.sin_family = AF_INET;
	bcopy((char*)hp->h_addr, (char*)&sinTCP.sin_addr.s_addr, hp->h_length);
	sinTCP.sin_port=htons(port);

	struct sockaddr_in sinUDP;
	bzero((char*)&sinUDP, sizeof(sinUDP));
	sinUDP.sin_family = AF_INET;
	bcopy((char*)hp->h_addr, (char*)&sinUDP.sin_addr.s_addr, hp->h_length);
	sinUDP.sin_port=htons(port);

	// create the socket
	int socketDescriptorTCP;
	if ((socketDescriptorTCP = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	// connect to server
	if (connect(socketDescriptorTCP, (struct sockaddr*)&sinTCP, sizeof(sinTCP)) < 0) {
		perror("Connection to server failed");
		close(socketDescriptorTCP);
		exit(1);
	}

	// UDP socket creation
	int socketDescriptorUDP;
	if((socketDescriptorUDP = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		cout << "socket creation unsuccessful" << endl;
		exit(1);
	}

	char buf[MAX_MESSAGE_LENGTH];
	int bufsize = sizeof(buf);

	sendMessageUDP(socketDescriptorUDP, &sinUDP, "init");

	while (true) {
		string username, password;
		recvMessageUDP(socketDescriptorUDP, buf, bufsize, &sinUDP);
		cout << buf;
		cin >> username;
		sendMessageUDP(socketDescriptorUDP, &sinUDP, username);

		recvMessageUDP(socketDescriptorUDP, buf, bufsize, &sinUDP);
		cout << buf;
		cin >> password;
		sendMessageUDP(socketDescriptorUDP, &sinUDP, password);

		recvMessageUDP(socketDescriptorUDP, buf, bufsize, &sinUDP);
		string response = buf;

		cout << buf << endl;

		if (response == "login success") {
			break;
		}
	}

	// main loop that interprets commands
	string command;
	while(1) {
		promptUserForOperation();
		cin >> command;

		sendMessageUDP(socketDescriptorUDP, &sinUDP, command);

		if (command == "CRT") {
			sendAndReceiveBoardRequest(socketDescriptorUDP, buf, bufsize, &sinUDP);
		} else if (command == "DLT") {
			deleteMessageFromBoard(socketDescriptorUDP, buf, bufsize, &sinUDP);
		} else if(command == "DST") {
			sendAndReceiveBoardRequest(socketDescriptorUDP, buf, bufsize, &sinUDP);
		} else if (command == "EDT") {
			editMessage(socketDescriptorUDP, buf, bufsize, &sinUDP);
		} else if (command == "LIS") {
			listBoards(socketDescriptorUDP, buf, bufsize, &sinUDP);
		} else if (command == "MSG") {
			addMessageToBoard(socketDescriptorUDP, buf, bufsize, &sinUDP);
		} else if (command == "RDB") {

		} else if (command == "APN") {
			
		} else if (command == "DWN") {

		} else if (command == "XIT") {
			close(socketDescriptorTCP);
			close(socketDescriptorUDP);
			exit(0);
		} else if (command == "SHT") {
			bool shutDown = shutdownServer(socketDescriptorUDP, buf, bufsize, &sinUDP);
			if (shutDown) {
				close(socketDescriptorUDP);
				close(socketDescriptorTCP);
				break;
			}
		} else {
			recvMessageUDP(socketDescriptorUDP, buf, bufsize, &sinUDP);
			cout << buf;
		}

		cout << endl;
	}

	return 0;
}

/* requests and displays a list of the board names from the server */
void listBoards(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP) {
	string boardname, messageNumber, newMessage;

	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << buf << endl;
}

/* allows the user to specify a message to edit */
void editMessage(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP) {
	string boardname, messageNumber, newMessage;

	// get board and message data to send to server
	cout << "board name: ";
	cin >> boardname;
	sendMessageUDP(socketDescriptorUDP, sinUDP, boardname);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << "message number: ";
	cin >> messageNumber;
	sendMessageUDP(socketDescriptorUDP, sinUDP, messageNumber);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << "new message: ";
	cin.ignore();
	getline(cin, newMessage);
	sendMessageUDP(socketDescriptorUDP, sinUDP, newMessage);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << buf << endl;
}

/* appends a file to the specified board if the board exists and the file isn't already appended to it */
void appendFileToBoard(int socketDescriptorUDP, int socketDescriptorTCP, char* buf, int bufsize, struct sockaddr_in* sinUDP) {
	string boardname, filename, result;

	// get and send board name
	cout << "board name: ";
	cin >> boardname;
	sendMessageUDP(socketDescriptorUDP, sinUDP, boardname);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	// get and send file name
	cout << "file name: ";
	cin >> filename;
	sendMessageUDP(socketDescriptorUDP, sinUDP, filename);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);
	result = buf;

	// if file cannot be appended, display result and return to prompt user for operation state
	if (result != "success") {
		cout << result << endl;
		return;
	}

	// file can be appended so check if it exists and its size
	if (!fileExists(filename)) {
		cout << "error: file does not exist" << endl;
		sendMessageUDP(socketDescriptorUDP, sinUDP, "abort");

		return;
	}

	// determine and send file size
	struct stat filestatus;
	stat(filename.c_str(), &filestatus);
	ostringstream oss;
	oss << filestatus.st_size;
	sendMessageUDP(socketDescriptorUDP, sinUDP, oss.str());

	// open file for reading
	FILE *fp;
	fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		cout << "error: file cannot be read and uploaded" << endl;
		exit(1);
	}

	// upload the file to the board
	int bytesSent = 0;
	int bytesRead;
	while (bytesSent < filestatus.st_size) {
		bytesRead = fread(buf, sizeof(char), bufsize, fp);
		bytesRead = sendMessageTCP(socketDescriptorTCP, buf, bytesRead);
		bzero(buf, bufsize);
		bytesSent += bytesRead;
	}
	fclose(fp);

}

/* delete the user specified message from the specified board, if it exists */
void deleteMessageFromBoard(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP) {
	string boardname, messageNumber;

	// get board and message data to send to server
	cout << "board name: ";
	cin >> boardname;
	sendMessageUDP(socketDescriptorUDP, sinUDP, boardname);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << "message number: ";
	cin >> messageNumber;
	sendMessageUDP(socketDescriptorUDP, sinUDP, messageNumber);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << buf << endl;
}

void addMessageToBoard(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in * sinUDP) {
	string boardname, message;

	// get board and message data to send to server
	cout << "board name: ";
	cin >> boardname;
	sendMessageUDP(socketDescriptorUDP, sinUDP, boardname);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << "message: ";
	cin.ignore();
	getline(cin, message);
	sendMessageUDP(socketDescriptorUDP, sinUDP, message);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);

	cout << buf << endl;
}

/* send & receive board data */
void sendAndReceiveBoardRequest(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in* sinUDP) {
	string boardname;
	cout << "board name: ";
	cin >> boardname;
	sendMessageUDP(socketDescriptorUDP, sinUDP, boardname);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);
	cout << buf << endl;
}

// shut down server. Returns false if shutdown was successful, true otherwise
bool shutdownServer(int socketDescriptorUDP, char* buf, int bufsize, struct sockaddr_in* sinUDP) {
	string adminPassword;
	cout << "admin password: ";
	cin >> adminPassword;

	sendMessageUDP(socketDescriptorUDP, sinUDP, adminPassword);
	recvMessageUDP(socketDescriptorUDP, buf, bufsize, sinUDP);
	string result = buf;

	if (result == "success") {
		return true;
	} else {
		cout << buf;
		return false;
	}
}

/* lists the possible operations for the message board application */
void promptUserForOperation() {
	cout << endl;
	cout << "Please choose one of the following commands:" << endl;
	cout << "CRT - create a new message board on the server" << endl;
	cout << "MSG - leave a message on a board" << endl;
	cout << "DLT - delete a message on a board" << endl;
	cout << "EDT - edit a message on a board" << endl;
	cout << "RDB - read a board" << endl;
	cout << "LIS - list the names of the board on the server" << endl;
	cout << "APN - append a file to a board" << endl;
	cout << "DWN - download a file to a board" << endl;
	cout << "DST - destroy a board" << endl;
	cout << "XIT - close connection to server and exit" << endl;
	cout << "SHT - shutdown the server and exit" << endl;
	cout << endl;
}

/* helper function to handle sending a message via udp. handles errors
 */
int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg) {
	const char* buf = msg.c_str();
	int msglen = msg.length();

	int bytesSent = sendto(socketDescriptor, buf, msglen, 0, (struct sockaddr*)sin, sizeof(struct sockaddr));

	if (bytesSent == -1) {
		perror("client failed to send to server");
		exit(1);
	}

	return bytesSent;
}

/* helper function to handle sending a message to a server via tcp. handles errors
 */
int sendMessageTCP(int socketDescriptor, string msg, int msglen) {
	int bytesSent;
	int totalBytesSent = 0;
	const char* buf = msg.c_str();

	while (totalBytesSent < msglen) {
		bytesSent = send(socketDescriptor, buf + totalBytesSent, msglen - totalBytesSent, 0);

		if (bytesSent == -1) {
			perror("client failed to send to server");
			exit(1);
		}

		totalBytesSent += bytesSent;
	}

	return totalBytesSent;
}

/* receive data udp style */
int recvMessageUDP(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin) {
	bzero(buf, bufsize);
	socklen_t addr_len = sizeof(*sin);

	int recvResult = recvfrom(socketDescriptor, buf, bufsize, 0, (struct sockaddr*)sin, &addr_len);
	if (recvResult == -1) {
		perror("server failed to receive from client");
		exit(1);
	}

	return recvResult;
}

/* receive data tcp style */
int recvMessageTCP(int socketDescriptor, char* buf, int bufsize) {
	bzero(buf, bufsize);

	int recvResult = recv(socketDescriptor, buf, bufsize, 0);
	if (recvResult == -1) {
		perror("server failed to receive from client");
		exit(1);
	}

	return recvResult;
}

/* determine if a file exists */
bool fileExists(string filename) {
	
	if (filename.length() == 0) return 0;

	FILE *fp;
	bool fileExists = false;

	fp = fopen(filename.c_str(), "r");
	if (fp != NULL) {
		fileExists = true;
		fclose(fp);
	}

	return fileExists;
}

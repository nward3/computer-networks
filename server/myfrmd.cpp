/*
 * Luke Garrison, Nick Ward
 * lgarriso, nward3
 *
 * server side for message board forum
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Board.h"
using namespace std;

#define MAX_MESSAGE_LENGTH 4096

int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg);
int sendMessageTCP(int socketDescriptor, string msg);
int recvMessageUDP(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);
int recvMessageTCP(int socketDescriptor, char* buf, int bufsize);
bool shutdownServer(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin, string adminPassword);
void createBoard(unordered_map<string, Board*> &boards, string username, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);
void deleteFile(string filename);
int stringToInt(string s);
bool boardExists(unordered_map<string, Board*> &boards, string boardname);
void destroyBoard(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin, string username, unordered_map<string, Board> &boards);
void addMessageToBoard(unordered_map<string, Board*> &boards, string username, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);
void deleteMessageFromBoard(unordered_map<string, Board*> &boards, string user, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);
void editMessage(unordered_map<string, Board*> &boards, string user, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin);

int main(int argc, char* argv[]) {

	// hash map for usernames and passwords
	unordered_map<string, string> users;

	// message boards - name of board to board itself
	unordered_map<string, Board*> boards;

	// check for proper function invocation
	if (argc != 3) {
		cout << "usage: ./myfrmd <port> <password>" << endl;
		exit(1);
	}

	// determine port number
	int port = atoi(argv[1]);
	if (port <= 0) {
		cout << "error: invalid port number" << endl;
		exit(1);
	}

	// determine admin password
	string adminPassword = argv[2];

	// build address data structure for udp
	struct sockaddr_in sinUDP;
	struct sockaddr_in clientaddr;
	bzero((char *) &sinUDP, sizeof(sinUDP));
	sinUDP.sin_family = AF_INET;
	sinUDP.sin_addr.s_addr = INADDR_ANY;
	sinUDP.sin_port = htons(port);

	// udp socket creation
	int socketDescriptorUDP;
	if ((socketDescriptorUDP = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		cout << "error: unable to create udp socket" << endl;
		exit(1);
	}

	// bind the udp socket to the specified address
	if (bind(socketDescriptorUDP, (struct sockaddr *) &sinUDP, sizeof(sinUDP)) < 0) {
		cout << "error: unable to bind socket" << endl;
		exit(1);
	}

	// build address data structure for udp
	struct sockaddr_in sinTCP;
	bzero((char *) &sinTCP, sizeof(sinTCP));
	sinTCP.sin_family = AF_INET;
	sinTCP.sin_addr.s_addr = INADDR_ANY;
	sinTCP.sin_port = htons(port);

	// tcp socket creation
	socklen_t len;
	int socketDescriptorTCP;
	if ((socketDescriptorTCP = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "error: unable to create tcp socket" << endl;
		exit(1);
	}

	// set socket option to allow for reuse
	int opt = 1;
	if ((setsockopt(socketDescriptorTCP, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))) < 0) {
		cout << "error: unable to set up socket for reuse" << endl;
		exit(1);
	}

	// bind the tcp socket to the specified address
	if (bind(socketDescriptorTCP, (struct sockaddr *) &sinTCP, sizeof(sinTCP)) < 0) {
		cout << "error: unable to bind socket" << endl;
		exit(1);
	}

	// open the passive socket by listening on the socket
	if (listen(socketDescriptorTCP, 1)) {
		cout << "error: unable to listen on socket" << endl;
		exit(1);
	}

	// data transfer buffer
	char buf[MAX_MESSAGE_LENGTH];
	int bufsize = sizeof(buf);
	string command;

	// wait for connection
	bool run = true;
	while(run) {
		
		// try to accept client connection on tcp socket
		int descriptor;
		if ((descriptor = accept(socketDescriptorTCP, (struct sockaddr *)&sinTCP, &len)) < 0) {
			cout << "error: unable to accept client connection" << endl;
			exit(1);
		}

		// initial client contact & determine clientaddr
		recvMessageUDP(socketDescriptorUDP, buf, bufsize, &clientaddr);

		// request username and password
		bool loggedOut = true;
		string user;
		while (loggedOut) {

			// send request for username
			sendMessageUDP(socketDescriptorUDP, &clientaddr, "username: ");

			// checks if new user or existing user and requests password
			recvMessageUDP(socketDescriptorUDP, buf, bufsize, &clientaddr);
			user = buf;
			bool newUser = false;

			auto search = users.find(user);
			if (search != users.end()) {
				sendMessageUDP(socketDescriptorUDP, &clientaddr, "password: ");
			} else {
				newUser = true;
				sendMessageUDP(socketDescriptorUDP, &clientaddr, "password: ");
			}
		
			// register new user or checks to see if the password matches
			recvMessageUDP(socketDescriptorUDP, buf, bufsize, &clientaddr);
			string password = buf;

			if (newUser) {
				users[user] = password;
				loggedOut = false;
			} else if (users[user] == password) {
				loggedOut = false;
			}
		}
		
		// send ACK on successful login
		sendMessageUDP(socketDescriptorUDP, &clientaddr, "login success");

		// receive and process client operations
		while(1) {

			recvMessageUDP(socketDescriptorUDP, buf, bufsize, &clientaddr);
			command = buf;

			if (command == "CRT") {
				createBoard(boards, user, socketDescriptorUDP, buf, bufsize, &clientaddr);
			} else if (command == "DLT") {
				deleteMessageFromBoard(boards, user, socketDescriptorUDP, buf, bufsize, &clientaddr);
			} else if (command == "EDT") {
				editMessage(boards, user, socketDescriptorUDP, buf, bufsize, &clientaddr);
			} else if (command == "MSG") {
				addMessageToBoard(boards, user, socketDescriptorUDP, buf, bufsize, &clientaddr);
			} else if (command == "XIT") {
				close(socketDescriptorUDP);
				close(socketDescriptorTCP);
				break;
			} else if (command == "SHT") {
				run = shutdownServer(socketDescriptorUDP, buf, bufsize, &clientaddr, adminPassword);
				
				if (!run) {
					close(socketDescriptorUDP);
					close(socketDescriptorTCP);

					break;
				}
			} else {
				sendMessageUDP(socketDescriptorUDP, &clientaddr, "Invalid command");
			}
		}
	}

	return 0;
}

/* edits the specified message if the board exists and the user has permission */
void editMessage(unordered_map<string, Board*> &boards, string user, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin) {
	string boardname, newMessage;
	int messageNumber;

	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	boardname = buf;
	sendMessageUDP(socketDescriptor, sin, "ACK");

	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	messageNumber = stringToInt(buf);
	sendMessageUDP(socketDescriptor, sin, "ACK");

	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	newMessage = buf;

	if (boardExists(boards, boardname)) {
		// try to delete the message from the board
		Board* board = boards[boardname];
		bool messageDeleted = board->editMessage(newMessage, messageNumber, user);
		if (messageDeleted) {
			sendMessageUDP(socketDescriptor, sin, "Message successfully updated");
		} else {
			sendMessageUDP(socketDescriptor, sin, "Unable to edit the message. You either don't have permission to edit it or the message doesn't exist");
		}
	} else {
		sendMessageUDP(socketDescriptor, sin, "The board '" + boardname + "' does not exist");
	}
}

/* delete the specified message from the board if the user is the one who posted it */
void deleteMessageFromBoard(unordered_map<string, Board*> &boards, string user, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin) {
	string boardname;
	int messageNumber;

	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	boardname = buf;
	sendMessageUDP(socketDescriptor, sin, "ACK");

	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	messageNumber = stringToInt(buf);

	if (boardExists(boards, boardname)) {
		// try to delete the message from the board
		Board* board = boards[boardname];
		bool messageDeleted = board->deleteMessage(messageNumber, user);
		if (messageDeleted) {
			sendMessageUDP(socketDescriptor, sin, "Message successfully removed");
		} else {
			sendMessageUDP(socketDescriptor, sin, "Unable to delete the message. You either don't have permission to delete it or the message doesn't exist");
		}
	} else {
		sendMessageUDP(socketDescriptor, sin, "The board '" + boardname + "' does not exist");
	}
}

/* add the received message to the specified board if the board exists */
void addMessageToBoard(unordered_map<string, Board*> &boards, string user, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin) {
	string boardname, message;
	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	boardname = buf;
	sendMessageUDP(socketDescriptor, sin, "ACK");

	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	message = buf;

	if (boardExists(boards, boardname)) {
		// add the message to the board
		Board* board = boards[boardname];
		board->addMessage(message, user);
		sendMessageUDP(socketDescriptor, sin, "Your message was added to board '" + boardname + "'");
	} else {
		sendMessageUDP(socketDescriptor, sin, "The board '" + boardname + "' does not exist");
	}
}

/* delete the specified file */
void deleteFile(string filename) {
	int status = remove(filename.c_str());
	if (status != 0) {
		cout << "error: failed to delete file" << endl;
		exit(1);
	}
}

// convert string to integer
int stringToInt(string s) {
	stringstream ss;
	ss << s;

	int i;
	ss >> i;

	return i;
}

// returns true if the board exists and false otherwise
bool boardExists(unordered_map<string, Board*> &boards, string boardname) {
	auto search = boards.find(boardname);
	return search != boards.end();
}

/* destroy the specified message board */
void destroyBoard(int socketDescriptor, char*buf, int bufsize, struct sockaddr_in* sin, string username, unordered_map<string, Board*> &boards) {
	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	string boardname = buf;

	// check if current user has permission to delete the board (it should be their board)
	auto search = boards.find(boardname);
	if (search != boards.end() && username == search->second->getUser()) {
		boards.erase(boardname);
		sendMessageUDP(socketDescriptor, sin, "success");
	} else {
		sendMessageUDP(socketDescriptor, sin, "error trying to delete board");
	}
}

/* create a new board in the message board forum */
void createBoard(unordered_map<string, Board*> &boards, string username, int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin) {
	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	string boardname = buf;

	auto search = boards.find(boardname);
	if (search == boards.end()) {
		boards[boardname] = new Board(boardname, username);
		sendMessageUDP(socketDescriptor, sin, "board successfully created");
	} else {
		sendMessageUDP(socketDescriptor, sin, "board creation failed");
	}
}

/* shutdown the server */
bool shutdownServer(int socketDescriptor, char* buf, int bufsize, struct sockaddr_in* sin, string adminPassword) {
	recvMessageUDP(socketDescriptor, buf, bufsize, sin);
	string password = buf;

	if (password == adminPassword) {
		sendMessageUDP(socketDescriptor, sin, "success");

		// TODO: delete all board files and all appended files

		return false;
	} else {
		sendMessageUDP(socketDescriptor, sin, "incorrect password");

		return true;
	}
}

/* send data udp style */
int sendMessageUDP(int socketDescriptor, struct sockaddr_in* sin, string msg) {
	const char* buf = msg.c_str();
	int msglen = msg.length();

	int bytesSent = sendto(socketDescriptor, buf, msglen, 0, (struct sockaddr*) sin, sizeof(struct sockaddr));

	if (bytesSent == -1) {
		perror("server failed to send to client");
		exit(1);
	}

	return bytesSent;
}

/* send data tcp style */
int sendMessageTCP(int socketDescriptor, string msg) {
	int bytesSent;
	int totalBytesSent = 0;
	const char* buf = msg.c_str();
	int msglen = msg.length();

	while (totalBytesSent < msglen) {
		bytesSent = send(socketDescriptor, buf + totalBytesSent, msglen - totalBytesSent, 0);

		if (bytesSent == -1) {
			perror("server failed to send to client");
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

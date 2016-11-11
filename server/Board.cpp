#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <cstring>

#include "Board.h"
#include "Message.h"

using namespace std;

Board::Board(string boardname, string createdByUser) {
	boardFileName = boardname;
	user = createdByUser;
}

Board::Board(const Board &board) {

}

// delete all attachments
Board::~Board() {

}

// add a message to the messages vector and write the message out to the board file
void Board::addMessage(string messageText, string username) {
	Message message(messageText, username);
	messages.push_back(message);
}

// returns true if removal was successful, ralse otherwise
bool Board::removeMessage(int messageNum, string user) {
	int messageIndex = messageNum - 1;

	if (messageIndex < 0 || (unsigned) messageIndex >= messages.size()) {
		return false;
	}
	else if (messages[messageIndex].getUser() != user) {
		// check that the user is the same user who posted the message
		return false;
	} else {
		// delete the message from the vector
		messages.erase(messages.begin() + messageIndex);

		return true;
	}
}

bool Board::fileExists (string filename) {
	if (filename.length() == 0) {
		return false;
	}

	FILE *pFile;
	bool fileExists = false;

	pFile = fopen(filename.c_str(), "r");

	if (pFile != NULL) {
		fileExists = true;
		fclose(pFile);
	} else {
		cout << "error: " << strerror(errno) << endl;
	}

	return fileExists;
}

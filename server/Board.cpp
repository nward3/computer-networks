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
	for (string filename : boardAttachments) {
		if (fileExists(filename)) {
			int status = remove(filename.c_str());
			if (status != 0) {
				cout << "error: failed to delete file: '" << filename << "'" << endl;
			}
		}
	}
}

// add a message to the messages vector and write the message out to the board file
void Board::addMessage(string messageText, string username) {
	Message message(messageText, username);
	messages.push_back(message);
}

// returns true if removal was successful, ralse otherwise
bool Board::deleteMessage(int messageNum, string user) {
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

// returns true if removal was successful, ralse otherwise
bool Board::editMessage(string newMessage, int messageNum, string user) {
	int messageIndex = messageNum - 1;

	if (messageIndex < 0 || (unsigned) messageIndex >= messages.size()) {
		return false;
	}
	else if (messages[messageIndex].getUser() != user) {
		// check that the user is the same user who posted the message
		return false;
	} else {
		// edit's the message's text
		messages[messageIndex].setMessageText(newMessage);

		return true;
	}
}

/* add the file name of an attachment to a list of the board's attachments */
void Board::appendBoardAttachment(string filename) {
	boardAttachments.push_back(filename);
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

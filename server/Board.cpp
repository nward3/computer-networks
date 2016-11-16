/*
 * Luke Garrison, Nick Ward
 * netIDs: lgarriso, nward3
 *
 * Implementation for Board class
 */

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

Board::Board(string boardName, string createdByUser) {
	this->boardName = boardName;
	this->user = createdByUser;
}

Board::Board(const Board &board) {

}

// delete all attachments
Board::~Board() {
	for (string attachmentName: boardAttachments) {
		string attachmentFileName = boardName + '-' + attachmentName;
		if (fileExists(attachmentFileName)) {
			int status = remove(attachmentFileName.c_str());
			if (status < 0) {
				cout << "error: failed to delete file: '" << attachmentFileName << "'" << endl;
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
void Board::appendBoardAttachment(string filename, string user) {
	boardAttachments.push_back(filename);

	// add a message to board stating original file name and user that attached it
	string messageToAdd = user + " appended the file: " + filename;
	addMessage(messageToAdd, user);
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

// write messages out to a specific temporary file
void Board::writeMessages() {
	ofstream ofs;
	ofs.open(getMessagesFileName().c_str());
	int messageCount = 1;

	for (auto message : getMessages()) {
		string msgtext = message.getMessageText();
		ofs << messageCount << ". " << msgtext << endl;
		messageCount++;
	}

	ofs.close();
}

string Board::getMessagesFileName() {
	return "messages-" + getBoardName();
}

// clean up after temporary messages file
void Board::deleteMessagesFile() {
	int status = remove(getMessagesFileName().c_str());
	if (status < 0) {
		perror("Error cleaning up temporary messages file");
	}
}

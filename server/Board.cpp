#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <cstring>

#include "Board.h"

using namespace std;

Board::Board(string boardname, string createdByUser) {
	boardFileName = boardname;
	user = createdByUser;

	// create board file stream and write out the user's name who created the board on the first line
	boardFileStream.open(boardname.c_str());
	boardFileStream << user << endl;
}

Board::Board(const Board &board) {

}

// close the file stream and delete the board file
Board::~Board() {
	boardFileStream.close();
	int status = remove(boardFileName.c_str());
	if (status < 0) {
		cout << "board file was unable to be deleted" << endl;
	}
}

// add a message to the messages vector and write the message out to the board file
void Board::addMessage(string message, string username) {
	messages.push_back(message);
	boardFileStream << message << "written by: " << username << endl;
}

// returns true if removal was successful, ralse otherwise
bool Board::removeMessage(int messageNum) {
	int messageIndex = messageNum - 1;

	if (messageIndex < 0 || (unsigned) messageIndex >= messages.size()) {
		return false;
	} else {
		// delete the message from the vector
		messages.erase(messages.begin() + messageIndex);
		
		// write user and all messages to file again
		boardFileStream.close();
		boardFileStream.open(boardFileName);
		boardFileStream << user << endl;

		for (auto msg : messages) {
			boardFileStream << msg << endl;
		}

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

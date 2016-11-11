#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>

#include "Message.h"

using namespace std;

class Board {
	public:
		Board(string boardname, string user);
		Board(const Board & board);
		~Board();
		void addMessage(string message, string username);
		bool removeMessage(int messageNum, string user);
		string getUser() { return user; }
		string getBoardFileName() { return boardFileName; }

	private:
		bool fileExists(string filename);

		vector<Message> messages;
		vector<string> filenames;
		string user;
		string boardFileName;
};

#endif

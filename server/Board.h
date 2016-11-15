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
		bool deleteMessage(int messageNum, string user);
		bool editMessage(string newMessage, int messageNum, string user);
		string getUser() { return user; }
		string getBoardName() { return boardName; }
		string getBoardFileName() { return getBoardName(); }
		void appendBoardAttachment(string filename, string user);
		vector<string> getBoardAttachments() { return boardAttachments; }
		vector<Message> getMessages() { return messages; }
		void writeMessages();
		string getMessagesFileName();
		void deleteMessagesFile();

	private:
		bool fileExists(string filename);

		vector<Message> messages;
		vector<string> boardAttachments;
		string user;
		string boardName;
};

#endif

#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include <fstream>

using namespace std;

class Board {
	public:
		Board(string boardname, string user);
		~Board();
		void addMessage(string message);
		bool removeMessage(int messageNum);

	private:
		bool fileExists(string filename);

		vector<string> messages;
		string user;
		ofstream boardFileStream;
		string boardFileName;
};

#endif

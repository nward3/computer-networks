#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

using namespace std;

class Message {
	public:
		Message(string messageText, string user);
		string getUser();
		string getMessageText();

	private:
		string user;
		string text;
};

#endif

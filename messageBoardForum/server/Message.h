/*
 * Luke Garrison, Nick Ward
 * netIDs: lgarriso, nward3
 *
 * Interface for Message class
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

using namespace std;

class Message {
	public:
		Message(string messageText, string user);
		string getUser();
		string getMessageText();
		void setMessageText(string newText);

	private:
		string user;
		string text;
};

#endif

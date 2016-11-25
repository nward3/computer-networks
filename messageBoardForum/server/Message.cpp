/*
 * Luke Garrison, Nick Ward
 * netIDs: lgarriso, nward3
 *
 * Implementation for Message class
 */

#include "Message.h"

Message::Message(string messageText, string userStr) {
	text = messageText;
	user = userStr;
}

string Message::getUser() {
	return user;
}

string Message::getMessageText() {
	return text;
}

void Message::setMessageText(string messageText) {
	text = messageText;
}

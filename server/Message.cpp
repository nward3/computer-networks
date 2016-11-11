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


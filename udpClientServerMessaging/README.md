# UDP Client-Server Messaging

This demonstrates a simple UDP client-server communication.

The UDP server program waits for a client to send a message to a port specified
from the command line. The server then appends timestamp with microsecond accuracy
to the end of the client's message and then encrypts the entire message according
to the key specified on the command line when starting the server. The server then
sends the encrypted message back to the client in one message and then sends the
encryption key to client in a separate message afterwards. The server then waits
for the next incoming client connection.

The UDP client program reads in a file or string from the command line, sends the
text to the server on the specified port, then receives a copy of the encrypted
text with the addition of the timestamp of when the server received the text. The
client then decrypts the message using the key received from the server. Finally,
the client displays the decrypted message from the server and the round trip time.


## Running

The udpserver can be run from the command line as follows:
```udpserver <port> <key>```, where port is the port number and
key is an encryption key of 10-15 alphanumeric characters.

Once the server is running, the client be be started from the command
line as follows: ```udpclient <host> <port> <text>```, where host is the
host server's name, port is the port number, and text is the name of the
file to send or a user provided message.

An example of how to run the UDP client-server where the server is
```student02.cse.nd.edu``` is as follows:
```udpserver 41025 abcdefghijk```.
Next run: ```udpclient student02.cse.nd.edu 41025 "text to send to server"```.

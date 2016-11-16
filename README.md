Nick Ward, Luke Garrison

netIDs: nward3, lgarriso

## Overview

This is a command-line message board forum application that includes both the client and server sides and utilizes UDP and TCP.

## Description

The files contained in this submission are README.md and directories server and
client. The server directory contains makefile and myfrmd.cpp. The client directory
contains makefile and myfrm.cpp.

README.md is the file containing the complete listing and explanation of what files
are present in the submission and the instructions to run the code.

The server directory's makefile is used to compile myfrmd.cpp, which is the forum's server. myfrmd.cpp is the source code file containing the server for the message board application. The server is started by specifying a port and an admin password and opens the specified port and waits for a client connection. Once a client connects to the server on the appropriate port, the server prompts the client for a username and password. If the username already exists, the password must match, or else the client is re-prompted. If it is a new user, the server will log the user in and record their password. The server will then wait for the client to specify an operation command. The server supports the following operation commands: CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT and SHT. The server will continue to handle operation commands from the client until the client ends the connection with the XIT command or the SHT command. If the client specifies the XIT command, the client will disconnect from the server, and the server will remain in a "wait for client connection" state. If the client specifies the SHT command and specifies the admin password, the server will delete all of the attachment files from all of the boards and shut down.

The client directory's makefile is used to compile myfrm.cpp, which is the 
client. Then running the client, the user specifies the host name for the server and 
the por it wants to connect on. Once the connection has been established, the client 
needs to log in by specifying a username and password. The client can then specify
one of the following operation commands: CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, 
DST, XIT and SHT.

## How To Run

An example of how to run this message board application with the server being
student02.cse.nd.edu would be to first start the server from the server directory as follows:
```./myfrmd 41025 password``` where 41025 is the port number to start the server on
and password is the admin password for the server.
Once the server is running, the client can be started by running the following
command from the client directory on the command line: ```myfrm student02.cse.nd.edu 41025```. With the
connection established, after the user logs in, the client is then able to run any of the operation
commands as long as the server is in a 'wait for operation from client' state.

* An example of the CRT command is:

  ```
  CRT
  <boardname>
  ```

  This tells the server to create the board with the specified name.

* An example of the MSG command is:

  ```
  MSG
  <boardname>
  <message text>
  ```

  This tells the server to create a message with the message text and
  add it to the board with the specified name.

* An example of the DLT command is:

  ```
  DLT
  <boardname>
  <message number>
  ```

  This tells the server to delete the message at the specified index
  for the specified board.

* An example of the EDT command is:

  ```
  EDT
  <boardname>
  <message number>
  <new message>
  ```

  This tells the server to edit the message at the specified index for
  the specified board and replace the old message with the new message.

* An example of the LIS command is:

  ```
  LIS
  ```

  This will list the names of all the boards.

* An example of the RDB command is:

  ```
  RDB
  <boardname>
  ```

  This command will return a list of the messages on the specified board, including file attachments.

* An example of the APN command is:

  ```
  APN
  <boardname>
  <filename>
  ```

  This first sends the APN command and then sends the name of the board to
  post the file to followed by the name of the file to post.

* An example of the DWN command is:

  ```
  DWN
  <boardname>
  <filename>
  ```

  This first sends the DWN command then sends the name of the board to download
  the file from and the name of the file to download.

* An example of the DST command is:

  ```
  DST
  <boardname>
  ```

  This command tells the server to delete the specified board and all messages and attachments on that board. Note that this command will only succeed if the user is the same user who created the board.

* An example of the XIT command is:

  ```
  XIT
  ```

  This will close the connection between the client and the server. The client will need
to reconnect with the server before trying to specify any additional operation commands.

* An example of the SHT command is:

  ```
  SHT
  <admin password>
  ```

  This will close the connection between the client and the server and the server will delete all boards and their attachments and will have to be manually restarted in order for the client to reconnect.

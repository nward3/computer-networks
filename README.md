Nick Ward, Luke Garrison

netIDs: nward3, lgarriso

The files contained in this submission are README.md and directories server and
client. The server directory contains makefile and myfrmd.cpp. The client directory
contains makefile and myfrm.cpp.

README.md is the file containing the complete listing and explanation of what files
are present in the submission and the instructions to run the code.

The server directory's makefile is used to compile myfrmd.cpp, which is the forum's server. myfrmd.cpp is the source code file containing the server for the FTP
application. The tcp server opens the specified port and waits for a client
connection. Once a tcp client connects to the server of the appropriate port, the
server waits for a client to specify an operation command. The server supports the
following operation commands: REQ, UPL, DEL, LIS, MKD, RMD, CHD, and XIT. The
server will continue to handle operation commands from the client until the client
ends the connection with XIT command. At this point, the server will return to a
state where it waits for the next client to connect on the specified port and will
continue to do this when a client closes its connection.

The client directory's makefile is used to compule myfrm.cpp, which is the tcp
client. myfrm.cpp is the source code file containing the client for the FTP
application. The tcp client specifies the host name for the server and the port
it wants to connect on. Once the connection has been establishedm, the client
can specify one of the following operation commands: REQ, UPL, DEL, LIS, MKD, RMD,
CHD, and XIT. The REQ command allows the client to download a specified file from
the server. The UPL command allows the client to upload a specified file to the
server. The DEL command allows the client to delete the specified file from the
server. The LIS command allows the client to see the contents (all files and
directories) of the current directory on the server. The MKD allows the client to
create a new subdirectory on the server. The CHD command allows the client to
change the current directory on the server. The RMD command allows the client to
remove a specific directory from the server if the specific directory is empty.
Finally, the XIT command closes the connection between the client and the server.

An example of how to run this message board application with the server being
student02.cse.nd.edu would be to first start the server from the server directory as follows:
```./myfrmd 41025 password``` where 41025 is the port number to start the server on
and password is the admin password for the server.
Once the server is running, the client can be started by running the following
command from the client directory on the command line: ```myfrm student02.cse.nd.edu 41025```. With the
connection established, the client is then able to run any of the operation
commands as long as the server is in a 'wait for operation from client' state.

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

* An example of the CRT command is:

  ```
  CRT
  <boardname>
  ```

  This tells the server to create the board with the specified name.

* An example of the LIS command is:

  ```
  LIS
  ```

  This will list the names of all the boards.

* An example of the MSG command is:

  ```
  MSG
  <boardname>
  <messagetext>
  ```

  This tells the server to create a message with the message text and
  add it to the board with the specified name.

* An example of the DLT command is:

  ```
  DLT
  <boardname>
  <messagenumber>
  ```

  This tells the server to delete the message at the specified index
  for the specified board.

* An example of the EDT command is:

  ```
  EDT
  <boardname>
  <messagenumber>
  <newmessage>
  ```

  This tells the server to edit the message at the specified index for
  the specified board and replace the old message with the new message.

* An example of the XIT command is:

  ```
  XIT
  ```

  This will close the connection between the client and the server. The client will need
to reconnect with the server before trying to specify any additional operation commands.

Nick Ward, Luke Garrison
netids: nward3, lgarriso

The files contained in this submission are README.md and directories server and
client. The server directory contains makefile and myftpd.cpp. The client directory
contains makefile and myftp.cpp.

README.md is the file containing the complete listing and explanation of what files
are present in the submission and the instructions to run the code.

The server directory's makefile is used to compile myftpd.cpp, which is the tcp
server. myftpd.cpp is the source code file containing the server for the FTP
application. The tcp server opens the specified port and waits for a client
connection. Once a tcp client connects to the server of the appropriate port, the
server waits for a client to specify an operation command. The server supports the
following operation commands: REQ, UPL, DEL, LIS, MKD, RMD, CHD, and XIT. The
server will continue to handle operation commands from the client until the client
ends the connection with XIT command. At this point, the server will return to a
state where it waits for the next client to connect on the specified port and will
continue to do this when a client closes its connection.

The client directory's makefile is used to compule myftp.cpp, which is the tcp
client. myftp.cpp is the source code file containing the client for the FTP
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

An example of how to run this ftp application with the server being
student02.cse.nd.edu would be to first start the server as follows:
```myftpd 41025``` where 41025 is the port number to start the connection on.
Once the server is running, the client can be started by running the following
command from the command line: ```myftp student02.cse.nd.edu 41025```. With the
connection established, the client is then able to run any of the operation
commands as long as the server is in a 'wait for operation from client' state.

An example of the REQ command is:
```REQ```
```remoteFile.txt```
This first sends the REQ command and then sends the name of the requested file to
the server.

An example of the UPL command is:
```UPL```
```fileToUpload.txt```
This first sends the UPL command then sends the name of the file to upload.

An example of the DEL command is:
```DEL```
```fileToDelete.txt```
This tells the server to delete the specified file.

An example of the LIS command is:
```LIS```
This will list all the files and subdirectories of the current directory on the server.

An example of the MKD command is:
```MKD```
```directoryToCreate```
This tells the server to create a new directory with the specified name.

An example of the RMD command is:
```RMD```
```emptyDirectoryToRemove```
This tells the server to remove the directory with the specified name (as long as the
directory is empty).

An example of the CHD command is:
```CHD```
```directoryToChangeTo```
This tells the server to change the working directory the directory specified.

An example of the XIT command is:
```XIT```
This will close the connection between the client and the server. The client will need
to reconnect with the server before trying to specify any additional operation commands.

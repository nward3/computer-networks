/* PG 3: Simple File Transfer Protocol
 * Name: Nick Ward & Luke Garrison
 * netids: nward3 & lgarriso */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <errno.h>
#include <unordered_set>
#include <sstream>
#include <mhash.h>
#include <ctime>
using namespace std;

#define MAX_MESSAGE_LENGTH 4096

int getDirectoryNameAndLength(string &directoryname, int socketDescriptor, char* buf, int bufsize);
void clearBuffer(char* buf, int bufsize);
int sendMessage(int socketDescriptor, string message);
int recvMessage(int socketDescriptor, char* buf, int bufsize);
bool isValidCommand(string command);
bool directoryExists(string directoryName);
bool fileExists(string fileName);
string getDirectoryListing(int socketDescriptor);
int createDirectory(string directoryName);
int changeDirectory(string directoryName);
void removeDirectory(string directoryName, int socketDescriptor, char* buf, int bufsize);
void deleteFile(string fileName, int socketDescriptor, char* buf, int bufsize);
void saveFile(string fileName, int socketDescriptor, char* buf, int bufsize);
void sendFile(string fileName, int socketDescriptor, char* buf, int bufsize);
string intToString(int i);
string longToString(long i);
int stringToInt(string bytes);
double timeElapsed(struct timeval* start, struct timeval* end);
double calculateThroughput(struct timeval* start, struct timeval* end, long bytesTranferred);
char* getFileHash(string fileName);
string getTransferResults(struct timeval* start, struct timeval* end, long fileSize);

int main(int argc, char* argv[]) {

	// define data structures to be used
	struct sockaddr_in server_addr;
	char buf[4096];
	int bufsize = sizeof(buf);
	socklen_t len;
	int socketfd, bytesReceived, socketDescriptor;

	// check for proper function invocation
	if (argc != 2) {
		cout << "usage: myftpd <port>" << endl;
		exit(1);
	}

	// determine port number
	int port = atoi(argv[1]);
	if (port <= 0) {
		cout << "error: invalid port number" << endl;
		exit(1);
	}

	// build address data structure
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	// tcp socket creation
	socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {
		cout << "error: unable to create socket" << endl;
		exit(1);
	}

	// set socket option to allow for reuse
	int opt = 1;
	if ((setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))) < 0) {
		cout << "error: unable set up socket for reuse" << endl;
		exit(1);
	}

	// bind the socket to the specified address
	if (bind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		cout << "error: unable to bind socket" << endl;
		exit(1);
	}

	// open the passive socket by listening on the socket
	if ((listen(socketfd, 1))) {
		cout << "error: unable to listen on socket" << endl;
		exit(1);
	}

	// allow for client to XIT and reconnect
	while(1) {
		
		// wait for client connection
		if ((socketDescriptor = accept(socketfd, (struct sockaddr *) &server_addr, &len)) < 0) {
			cout << "error: unable to accept client connection" << endl;
			exit(1);
		} else {
			cout << "new connection established" << endl;
		}

		// receive and process client message
		while(1) {
			// initialize frequently used variables
			string directoryName;
			string fileName;

			bytesReceived = recvMessage(socketDescriptor, buf, bufsize);

			if (bytesReceived < 0) {
				cout << "error: unable to receive client's message" << endl;
				exit(1);
			} else if (bytesReceived > 0) {
				cout << "TCP server received: " << buf << endl;

				// process client command
				string command = buf;

				clearBuffer(buf, bufsize);

				if (command == "XIT") {
					close(socketDescriptor);
					break;
				} else if (command == "REQ") {
					if(getDirectoryNameAndLength(fileName, socketDescriptor, buf, bufsize) >= 0) {
						sendFile(fileName, socketDescriptor, buf, bufsize);
					}
				} else if (command == "UPL") {
					if (getDirectoryNameAndLength(fileName, socketDescriptor, buf, bufsize) >= 0) {
						sendMessage(socketDescriptor, "READY");
						saveFile(fileName, socketDescriptor, buf, bufsize);
					}
				} else if (command == "DEL") {
					if (getDirectoryNameAndLength(fileName, socketDescriptor, buf, bufsize) >= 0) {
						deleteFile(fileName, socketDescriptor, buf, bufsize);
					}
				} else if (command == "LIS") {
					string dirlisting = getDirectoryListing(socketDescriptor);
					sendMessage(socketDescriptor, dirlisting);
				} else if (command == "RMD") {
					if (getDirectoryNameAndLength(directoryName, socketDescriptor, buf, bufsize) >= 0) {
						removeDirectory(directoryName, socketDescriptor, buf, bufsize);
					}
				} else if (command == "MKD") {
					if (getDirectoryNameAndLength(directoryName, socketDescriptor, buf, bufsize) >= 0) {
						int status = createDirectory(directoryName);
						sendMessage(socketDescriptor, intToString(status));
					}
				} else if (command == "CHD") {
					if (getDirectoryNameAndLength(directoryName, socketDescriptor, buf, bufsize) >= 0) {
						int status = changeDirectory(directoryName);
						sendMessage(socketDescriptor, intToString(status));
					}
				} else {
					sendMessage(socketDescriptor, "Invalid FTP command");
				}
			}	
		}
	}

	free(buf);
}

// downloads file from the server to the client
void sendFile(string fileName, int socketDescriptor, char* buf, int bufsize) {
	// send file size if it exists otherwise send failure message
	if (!fileExists(fileName)) {
		sendMessage(socketDescriptor, "-1");
		return;
	}

	// determine size of file (in bytes) to send	
	struct stat filestatus;
	stat(fileName.c_str(), &filestatus);

	// send file size to client
	ostringstream oss;
	oss << filestatus.st_size;
	sendMessage(socketDescriptor, oss.str());

	// open file for reading
	FILE *fp;
	fp = fopen(fileName.c_str(), "rb");
	if (!fp) {
		exit(1);
	}

	// send file
	int bytesSent = 0;
	int bytesRead;
	while (bytesSent < filestatus.st_size) {
		bytesRead = fread(buf, sizeof(char), bufsize, fp);
		sendMessage(socketDescriptor, buf);
		clearBuffer(buf, bufsize);
		bytesSent += bytesRead;
	}
	fclose(fp);

	// wait for client to be ready for md5 hash
	recvMessage(socketDescriptor, buf, bufsize);

	char* hash = getFileHash(fileName);
	sendMessage(socketDescriptor, hash);

	free(hash);

	cout << "bytesSent: " << bytesSent << endl;
}

// uploads file from the client to the server
void saveFile(string fileName, int socketDescriptor, char* buf, int bufsize) {
	// receive size (in bytes) of file
	recvMessage(socketDescriptor, buf, bufsize);
	int totalBytes = stringToInt(buf);
	int bytesWritten = 0;

	// open file for writing
	FILE* fp;
	fp = fopen(fileName.c_str(), "wb");
	if (!fp) {
		exit(1);
	}

	// time from start of transfer to completion
	//
	struct timeval start;
	struct timeval end;
	gettimeofday(&start, NULL);
	// write to the open file
	int bytesReceived;
	while (bytesWritten < totalBytes) {
		bytesReceived = recvMessage(socketDescriptor, buf, bufsize);
		fwrite(buf, sizeof(char), bytesReceived, fp);
		bytesWritten += bytesReceived;
	}
	gettimeofday(&end, NULL);
	fclose(fp);

	// tell client to send hash
	sendMessage(socketDescriptor, "Ready for MD5 Hash");
	
	// receive hash from client
	recvMessage(socketDescriptor, buf, bufsize);
	string clientHash = buf;

	// compute MD5 hash for file transferred
	struct stat filestatus;
	stat(fileName.c_str(), &filestatus);

	char* hash = getFileHash(fileName);

	// check that client and server hash is same
	if (string(hash) == clientHash) {
		// calculate throughput and send it to client
		string transferResults = getTransferResults(&start, &end, bytesWritten);
		sendMessage(socketDescriptor, transferResults);
	} else {
		// clean up failed file transfer
		if (fileExists(fileName)) {
			remove(fileName.c_str());
		}
		sendMessage(socketDescriptor, "-1");
	}

	free(hash);
}

// deletes file
void deleteFile(string fileName, int socketDescriptor, char* buf, int bufsize) {
	bool result = fileExists(fileName);
	if (result) {
		sendMessage(socketDescriptor, "1");
		recvMessage(socketDescriptor, buf, bufsize);
		string choice = buf;

		if (choice == "Yes") {
			int status = remove(fileName.c_str());
			if (status == 0) {
				sendMessage(socketDescriptor, "1");
			} else {
				sendMessage(socketDescriptor, "-1");
			}
		}
	} else {
		sendMessage(socketDescriptor, "-1");
	}
}

// remove directory
void removeDirectory(string directoryName, int socketDescriptor, char* buf, int bufsize) {
	bool result = directoryExists(directoryName);
	if (result) {
		sendMessage(socketDescriptor, "1");
		recvMessage(socketDescriptor, buf, bufsize);
		string choice = buf;

		if (choice == "Yes") {
			int status = rmdir(directoryName.c_str());
			if (status == 0) {
				sendMessage(socketDescriptor, "1");
			} else {
				sendMessage(socketDescriptor, "-1");
			}
		}
	} else {
		sendMessage(socketDescriptor, "-1");
	}
}

// helper function to convert an integer to string
string intToString(int i) {
	stringstream ss;
	ss << i;

	return ss.str();
}

// convert long to string
string longToString(long i) {
	stringstream ss;
	ss << i;

	return ss.str();
}

// convert string to integer
int stringToInt(string s) {
	stringstream ss;
	ss << s;

	int i;
	ss >> i;

	return i;
}

/* Awaits directory name and length from client (concatenated in a single request)
 * Sets the directoryName or errorMessage string because they are passed by reference
 * returns 0 if there is no problem
 * returns -1 and sends error message to the client if there was an error
 */
int getDirectoryNameAndLength(string &directoryName, int socketDescriptor, char* buf, int bufsize) {
	recvMessage(socketDescriptor, buf, bufsize);
	string directoryData = buf;

	// parse dirlen and dirname from directoryData string
	unsigned dirlen;
	stringstream ss(directoryData);
	ss >> dirlen;
	if (ss.fail()) {
		sendMessage(socketDescriptor, "Directory length was not sent to server");
		return -1;
	}

	ss >> directoryName;

	if (dirlen != directoryName.length()) {
		sendMessage(socketDescriptor, "Directory name was corrupted when sent to the server");
		return -1;
	}

	return 0;
}

// clear the recv buffer
void clearBuffer(char* buf, int bufsize) {
	bzero(buf, bufsize);
}

// create directory if it does not exist
int changeDirectory(string directoryName) {
	if (directoryExists(directoryName)) {
		// try to create directory
		if (chdir(directoryName.c_str()) < 0) {
			return -1;
		} else {
			return 1;
		}
	} else {
		return -2;
	}
}

// create directory if it does not exist
int createDirectory(string directoryName) {
	if (directoryExists(directoryName)) {
		return -2;
	} else {
		// try to create directory
		if (mkdir(directoryName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
			return -1;
		} else {
			return 1;
		}
	}
}

// check if directory already exists
bool directoryExists (string pathname) {

	if (pathname.length() == 0) return false;
	
	DIR *pDir;
	bool dirExists = false;

	pDir = opendir(pathname.c_str());

	if (pDir != NULL) {
		dirExists = true;
		closedir(pDir);								
	} else {
		cout << "error: " << strerror(errno) << endl;
	}

	return dirExists;
}

// check if file exists
bool fileExists (string filename) {
	
	if (filename.length() == 0) return false;

	FILE *pFile;
	bool fileExists = false;

	pFile = fopen(filename.c_str(), "r");

	if (pFile != NULL) {
		fileExists = true;
		fclose(pFile);
	} else {
		cout << "error: " << strerror(errno) << endl;
	}

	return fileExists;
}

/* returns a string containing the files and directories in the current
 * directory each file or directory is separated by a newline */
string getDirectoryListing(int socketDescriptor) {
	DIR *dir;
	struct dirent *ent;
	string dirlisting;

	if ((dir = opendir (".")) != NULL) {
		/* iterate over all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			dirlisting += (ent->d_name + string("\n"));
		}
		closedir(dir);

		return dirlisting;
	} else {
		/* could not open directory */
		return string(strerror(errno));
	}
}

// sends message to server. Includes error checking
int sendMessage(int socketDescriptor, string message) {
	int sendResult = send(socketDescriptor, message.c_str(), message.length(), 0);
	if (sendResult == -1) {
		perror("client failed to send to server");
		exit(1);
	}
	
	return sendResult;
}

// sends message to server. Includes error checking
int recvMessage(int socketDescriptor, char* buf, int bufsize) {
	// clear buffer before receiving message
	clearBuffer(buf, bufsize);

	int recvResult = recv(socketDescriptor, buf, bufsize, 0);
	if (recvResult == -1) {
		perror("client failed to send receive data from server");
		exit(1);
	}

	return recvResult;
}

// returns true if command is valid
bool isValidCommand(string command) {
	unordered_set<string> validCommands = {"REQ", "UPL", "DEL", "LIS", "MKD", "RMD", "CHD", "XIT"};

	return validCommands.find(command) != validCommands.end();
}

// calculate time (in usec) elapsed from start to end
double timeElapsed(struct timeval* start, struct timeval* end) {
        struct timeval timeDiff;
        timersub(end, start, &timeDiff);

        return timeDiff.tv_sec + (double)timeDiff.tv_usec / 1000000;
}

// calculate throughput for file transfer in Megabytes / sec
double calculateThroughput(struct timeval* start, struct timeval* end, long bytesTransferred) {

        // calculate time elapsed from start to end
        double time = timeElapsed(start, end);

        // convert bytes to bits and calculate throughput in Megabytes / sec
        return bytesTransferred / time / 1000000;
}

string getTransferResults(struct timeval* start, struct timeval* end, long fileSize) {
        double throughput = calculateThroughput(start, end, fileSize);

		stringstream iss;
        iss << longToString(fileSize) << " bytes transferred in " << timeElapsed(start, end) << " seconds: " << throughput << " Megabytes/sec" << endl;

        return iss.str();
}

char* getFileHash(string fileName) {
	MHASH td;
	unsigned char filechar;
	FILE *fPtr;

	fPtr = fopen(fileName.c_str(), "rb");
	td = mhash_init(MHASH_MD5);
	if (td == MHASH_FAILED) {
			exit(1);
	}

	while (fread(&filechar, sizeof(char), 1, fPtr) == 1) {
			mhash(td, &filechar, 1);
	}

	fclose(fPtr);

	unsigned char* hash = (unsigned char*)malloc(sizeof(char*) * 16);

	mhash_deinit(td, hash);

	return (char*)hash;
}

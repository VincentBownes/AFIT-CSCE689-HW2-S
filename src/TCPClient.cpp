#include "TCPClient.h"
#include "exceptions.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h> 
#include <sys/types.h>
#include <cstddef> 
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <strings.h>
#include <string.h>
#include <string>

/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() {
}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) {
    client = socket(AF_INET, SOCK_STREAM, 0);
    if(client < 0){ 
        throw socket_error(std::string("failed to create socket ") + strerror(errno)); 
    }

    sockaddr_in clientInfo;
    clientInfo.sin_family = AF_INET;
    clientInfo.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &clientInfo.sin_addr);
    
    int retV = connect(client, (sockaddr*)&clientInfo, sizeof(clientInfo));
    if(retV < 0){ 
        throw socket_error(std::string("failed to connect socket ") + strerror(errno)); 
    }
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() {
    fd_set fileDescriptors;
    FD_ZERO(&fileDescriptors);
    FD_SET(client, &fileDescriptors);
    FD_SET(0, &fileDescriptors);

   while(true){
    fd_set localFDset = fileDescriptors;
    int socketCount = select(FD_SETSIZE, &localFDset, nullptr, nullptr, nullptr);
    //check if connection is still open
    char buffer[1024];
    if (recv(client, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == 0){
        break;
    }
    if(socketCount > 0){
        for(int i=0; i<FD_SETSIZE; i++){
            if(FD_ISSET(i, &localFDset)){
			    if(i == client){
                    char incomingMessage[1025];
                    memset(incomingMessage, 0, 1025);
                    ssize_t size = read(i, incomingMessage, 1024); 
                    std::cout << (char *)incomingMessage << "\n";
                }
                else{
                    char outgoingMessage[1025];
                    memset(outgoingMessage, 0, 1025);
                    ssize_t size = read(i, outgoingMessage, 1024); 
                    int n = write(client,outgoingMessage,strlen(outgoingMessage));
                }
            }
        }
    }
    else{
        break;
    }
    }
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {
    int retV = close(client);
    if(retV < 0){ 
        throw socket_error(std::string("failed to close socket ") + strerror(errno)); 
    }
}



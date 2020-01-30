#include "TCPServer.h"
#include "TCPConn.h"
#include "exceptions.h"
#include "singletonHelperFunctions.h"
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
#include <thread>
#include <chrono>
#include <cstring>
#include <map>

TCPServer::TCPServer() {

}

TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {
    listener = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(listener < 0){ 
        throw socket_error(std::string("failed to create socket ") + strerror(errno)); 
    } //if program fails to create server socket throw exception

    sockaddr_in listenerInfo;
    listenerInfo.sin_family = AF_INET;
    listenerInfo.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &listenerInfo.sin_addr);
    
    int retV = bind(listener, (sockaddr*)&listenerInfo, sizeof(listenerInfo));
    if(retV < 0){ 
        throw socket_error(std::string("failed to create socket ") + strerror(errno)); 
    }//if program fails to bind server socket throw exception

    singletonHelperFunctions::log("\nserver started up", 18);
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {
    int retV = listen(listener, SOMAXCONN); 
    if(retV < 0){ 
        throw socket_error(std::string("server failed to listen ") + strerror(errno)); 
    } // if server fails to listen throw exception

    std::map<int, TCPConn> connections;
    fd_set fileDescriptors;
    FD_ZERO(&fileDescriptors);
    FD_SET(listener, &fileDescriptors);
    
    while(true){
        fd_set localFDset = fileDescriptors;
        std::vector<int> keys;

        //Find all key values in map, keys are the file descriptor numbers
        for(std::map<int,TCPConn>::iterator it = connections.begin(); it != connections.end(); ++it) {
            keys.push_back(it->first);
        }//https://stackoverflow.com/questions/110157/how-to-retrieve-all-keys-or-values-from-a-stdmap-and-put-them-into-a-vector
        
        //use keys to check if all are still open, if not clear those that are not
        for(int i: keys){
            if (!connections[i].isConnected()){
                FD_CLR(i, &localFDset);
                FD_CLR(i, &fileDescriptors);
                connections.erase(i);
                keys.clear();

                //only need to redo if an fd was removed
                for(std::map<int,TCPConn>::iterator it = connections.begin(); it != connections.end(); ++it) {
                    keys.push_back(it->first);
                }
            }
        }
        keys.push_back(listener);

        int socketCount = select(FD_SETSIZE, &localFDset, nullptr, nullptr, nullptr);
        if(socketCount > 0){
		    for (int i: keys)
		    {
			    if(FD_ISSET(i, &localFDset)){
				    if(i == listener){
                        TCPConn tp;
                        int clientID = tp.acceptFunc(i);
                        if(clientID > 0){
                            bool auth = tp.startAuthentication();
                            if(auth){
                                connections.emplace(clientID, tp);
                                FD_SET(clientID, &fileDescriptors);
                                tp.sendMenu();
                            }
                        }
				    }
                    else{
                        connections[i].handleConnection();
                    }
			    }
		    }
        }
        else{ 
            std::cout << "select function returned -1 " << strerror(errno) << "\n";
            break;
        }
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
			
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
    int retV = close(listener);
    if(retV < 0){ 
        throw socket_error(std::string("failed to close socket ") + strerror(errno)); 
    } // if socket fails to close
}

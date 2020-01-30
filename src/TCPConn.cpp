#include "TCPConn.h"
#include "exceptions.h"
#include "strfuncts.h"
#include "singletonHelperFunctions.h"
#include <algorithm>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdexcept>
#include <sys/select.h> 
#include <sys/types.h>
#include <cstddef> 
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <strings.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

TCPConn::TCPConn(){

}

TCPConn::~TCPConn(){

}

//accept the client connection and assign the fd to connfd
int TCPConn::acceptFunc(int server){
    sockaddr_in clientAddr {};
    socklen_t clientAddrLen = (socklen_t)sizeof(clientAddr);
    connfd = accept(server, (struct sockaddr *)&clientAddr, &clientAddrLen);
	this-> ipString = inet_ntoa(clientAddr.sin_addr);
	if(connfd < 0){
		std::cout << "failed to connect " << strerror(errno) << "\n";
		return -1;
	}
	try{
		//check if new client is on whitelist. If not, disconnect
		bool onList = singletonHelperFunctions::checkWhitelist(clientAddr);
		if(onList != 1){ 
			singletonHelperFunctions::log("\nunidentified IP tried to connect: ", 35);		
			singletonHelperFunctions::log(this->ipString.c_str(), (int)this->ipString.length());
			this->disconnect(); 
			return -1;
		}
		singletonHelperFunctions::log("\nRecognized IP tried to connect: ", 33);
		singletonHelperFunctions::log(this->ipString.c_str(), (int)this->ipString.length());
		return connfd;
	}
	catch(logfile_error &e) {
      	std::cerr << "initialization failed: " << e.what() << std::endl;
	  	this->disconnect();
      	return -1;
    }	   	
}

//once connected retrieve the commands from the client and attempt to process them
void TCPConn::handleConnection(){
	char incomingMessage[1025];
    memset(incomingMessage, 0, 1025);
    ssize_t size = read(this->connfd, incomingMessage, 1024); 
	if(size < 0){
		return;
	}//if read returns an error 

	std::string nextCmd = "";
	std::stringstream cmds(incomingMessage);
	//
    while(std::getline(cmds,nextCmd,'\n')){
		commands.push_back(nextCmd);
  	}//https://stackoverflow.com/questions/13172158/c-split-string-by-line

	this->getMenuChoice();
}

//Execute commands in the order they are received
void TCPConn::getMenuChoice(){
	while(!this->commands.empty()){
		std::string cmd = commands.back();
		cmd.erase(std::remove(cmd.begin(), cmd.end(), ' '), cmd.end());
		commands.pop_back();
		if(cmd.compare("1") == 0) { this->sendText("16 Dec", 6); } 
		else if(cmd.compare("2") == 0) { this->sendText("0700", 4); } 
		else if(cmd.compare("3") == 0) { this->sendText("Snow", 4); } 
		else if(cmd.compare("4") == 0) { this->sendText("Full", 4); } 
		else if(cmd.compare("5") == 0) { this->sendText("slack", 5); }
		else if(cmd.compare("hello") == 0) { this->sendText("Welcome to the server!", 22); }
		else if(cmd.compare("menu") == 0) { this->sendMenu(); } 
		else if(cmd.compare("exit") == 0) { 
			singletonHelperFunctions::log("\n", 1);
			singletonHelperFunctions::log(this->username.c_str(), (int)this->username.length());
			singletonHelperFunctions::log(" disconnected with ip: ", 23);
			singletonHelperFunctions::log(this->ipString.c_str(), (int)this->ipString.length());
			this->disconnect(); 
		} 
		else if(cmd.compare("passwd") == 0) { this->changePassword(); } 
		else { this->sendText("Invalid option. Type 'menu' to see the list of available options", 64); } 
	}
}

int TCPConn::sendText(const char *msg, int size){
	char sendbuffer[size+1];
    snprintf(sendbuffer, sizeof(sendbuffer), "%s", msg);
    int ret = write(this->connfd, sendbuffer, strlen(sendbuffer));
	return ret;
}


void TCPConn::sendMenu(){
    std::string s = "<=Welcome to the server!" ; //22 chars long
	for (int i = 0; i < 82; i++) {
		s += "=";
	}
	s += "=>\n[";
    for (int i = 0; i < 106; i++) {
		s += " ";
	}
    s += "]\n";
    s += "[OPTIONS: 1-Date 2-Time 3-Weather 4-Moon phase 5-Tide"; //52
	for (int i = 0; i < 54; i++) {
		s += " ";
	}
	s += "]\n";
    s += "[ADDITIONAL OPTIONS: 'passwd'-Change your password 'exit'-Disconnect from server 'menu'-Display main menu  ]\n[";
    for (int i = 0; i < 106; i++) {
		s += " ";
	}
    s += "]\n";
    s += "<";
	for (int i = 0; i < 106 ; i++) {
		s += "=";
	}
	s += ">";

    //send menu to client
	int len = s.length();
    this->sendText(s.c_str(), len);
}

void TCPConn::disconnect(){
	int retV = close(this->connfd);
	connfd = -1;
	if(retV < 0){ 
        throw socket_error(std::string("failed to close socket ") + strerror(errno)); 
    }
}

bool TCPConn::isConnected(){
	if(connfd < 0){
		return false;
	}
	return true;
}


//first get username and check then password
bool TCPConn::startAuthentication(){
	this->getUsername();
	if(!singletonHelperFunctions::checkUsername(this->username.c_str())){
		singletonHelperFunctions::log("\n", 1);
		singletonHelperFunctions::log(this->username.c_str(), (int)this->username.length());
		singletonHelperFunctions::log(" tried to connect with unrecognized username and ip: ", 53);
		singletonHelperFunctions::log(this->ipString.c_str(), (int)this->ipString.length());
		this->disconnect();
		return false;
	}

	if(!this->getPasswd()){
		singletonHelperFunctions::log("\n", 1);
		singletonHelperFunctions::log(this->username.c_str(), (int)this->username.length());
		singletonHelperFunctions::log(" entered wrong password twice with ip: ", 39);
		singletonHelperFunctions::log(this->ipString.c_str(), (int)this->ipString.length());
		this->disconnect();
		return false;
	}

	singletonHelperFunctions::log("\n", 1);
	singletonHelperFunctions::log(this->username.c_str(), (int)this->username.length());
	singletonHelperFunctions::log(" successfully connected with ip: ", 33);
	singletonHelperFunctions::log(this->ipString.c_str(), (int)this->ipString.length());
	return true;
}

//get input from user and set username
void TCPConn::getUsername(){
	
    this->sendText("\nUsername: ", 12);

	char incomingMessage[1025];
    memset(incomingMessage, 0, 1025);
    ssize_t size = read(this->connfd, incomingMessage, 1024); 
	if(size < 0){
		return;
	}//if read returns an error
	std::string uname(incomingMessage);
    clrNewlines(uname);      
	
	this->username = uname;

}

//get input from user and call password helper function
bool TCPConn::getPasswd(){
	
	for(int i = 0; i < 2; i++){
		this->sendText("\nPassword: ", 12);

		char incomingMessage[1025];
    	memset(incomingMessage, 0, 1025);
    	ssize_t size = read(this->connfd, incomingMessage, 1024); 
		if(size < 0){
			return false;
		}//if read returns an error
		std::string pass(incomingMessage);
    	clrNewlines(pass);     

		bool check =  singletonHelperFunctions::checkPasswd(this->username.c_str(), pass.c_str());
		if(check){ return check; }
	}
	return false;
}


//change the users password
void TCPConn::changePassword(){
	this->sendText("\nNew Password: ", 16);

	char incomingMessage[1025];
    memset(incomingMessage, 0, 1025);
    ssize_t size = read(this->connfd, incomingMessage, 1024); 
	if(size < 0){
		return;
	}//if read returns an error
	std::string pass(incomingMessage);
    clrNewlines(pass);     

	this->newPwd = pass;

	this->sendText("\nEnter New Password Again: ", 28);

    memset(incomingMessage, 0, 1025);
    size = read(this->connfd, incomingMessage, 1024); 
	if(size < 0){
		return;
	}//if read returns an error
	std::string pass2(incomingMessage);
    clrNewlines(pass2);     

	if(this->newPwd.compare(pass) == 0){
		std::cout << "entering change pass func \n";
		bool check = singletonHelperFunctions::changePasswd(this->username.c_str(), pass2.c_str());
		std::cout << __LINE__ << "\n";
	}

}
#ifndef TCPCONN_H
#define TCPCONN_H
#pragma once

#include<string>
#include<vector>
#include<iostream>

//const int max_attempts = 2;

class TCPConn 
{
public:
   TCPConn();
   ~TCPConn();

   int acceptFunc(int server);

   int sendText(const char *msg, int size);

   void handleConnection();
   bool startAuthentication();
   void getUsername();
   bool getPasswd();
   void sendMenu();
   void getMenuChoice();
   //void setPassword();
   void changePassword();

   void disconnect();
   bool isConnected();

private:

   int connfd;

   std::string ipString;

   std::vector<std::string> commands;
 
   std::string username; // The username this connection is associated with

   std::string newPwd; // Used to store user input for changing passwords

   int pwdAttempts = 0;
};


#endif

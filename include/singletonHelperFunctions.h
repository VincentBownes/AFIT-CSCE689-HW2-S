#pragma once
#include <string>
#include <map>
#include <sys/socket.h> 
#include <netinet/in.h>
#include "PasswdMgr.h"


class singletonHelperFunctions
{
public:
   static bool checkWhitelist( sockaddr_in& ip );
   static bool checkUsername( const char* name );
   static bool checkPasswd( const char* name, const char* pass );
   static bool changePasswd( const char* name, const char* pass );
   static void log(const char* s, int n);

private:
   singletonHelperFunctions(); //default constructor
   static singletonHelperFunctions& instance();
   singletonHelperFunctions( const singletonHelperFunctions& ) = delete; //copy constructor
   singletonHelperFunctions( singletonHelperFunctions&& ) = delete; //move constructor
   singletonHelperFunctions& operator=( const singletonHelperFunctions& ) = delete; //assignment operator
   singletonHelperFunctions& operator=( singletonHelperFunctions&& ) = delete; //move assignment operator
   
   static singletonHelperFunctions* s; //declared here, but memory storage goes only in one translation unit
   PasswdMgr* pwd;
   
   std::map< std::string, std::string > map;
   const std::string filePath = "whitelist.txt";
};
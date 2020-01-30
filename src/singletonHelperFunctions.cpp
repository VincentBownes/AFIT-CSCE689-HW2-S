#include "singletonHelperFunctions.h"
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fstream>

//singleton patter used so that only one instance of the password manager is ever needed

singletonHelperFunctions* singletonHelperFunctions::s = nullptr;

//create password manager
singletonHelperFunctions::singletonHelperFunctions() {
    this->pwd = new PasswdMgr("passwd");
}

singletonHelperFunctions& singletonHelperFunctions::instance()
{
   if( singletonHelperFunctions::s == nullptr )
      singletonHelperFunctions::s = new singletonHelperFunctions();
   return *singletonHelperFunctions::s;
}

bool singletonHelperFunctions::checkWhitelist( sockaddr_in& ip ){
    
    //convert ip to a string
    std::string dotIP = inet_ntoa(ip.sin_addr);

    //print the ip of the client that is trying to connect  
    std::cout << dotIP << "\n";

    //get the filepath to the whitelist file and compare all entries against ip
    auto path = singletonHelperFunctions::instance().filePath;
    std::ifstream infile(path);
    std::string s;
    if(infile){
        while(infile >> s){
            if(s.compare(dotIP) == 0){
                return true;
            }
        }
    }
    else{ throw logfile_error(std::string("failed to open whiteList file ") );  }
    
    return false;
}

//use password manager to check username
bool singletonHelperFunctions::checkUsername(const char* name){

    bool check = singletonHelperFunctions::instance().pwd->checkUser(name);
    return check;
}

//use password manager to check password
bool singletonHelperFunctions::checkPasswd(const char* name, const char* pass){

    bool check = singletonHelperFunctions::instance().pwd->checkPasswd(name, pass);
    return check;
}

//use password manager to change password
bool singletonHelperFunctions::changePasswd( const char* name, const char* pass ){
    bool check = singletonHelperFunctions::instance().pwd->changePasswd(name, pass);
    return check;
}

//log events into a file
void singletonHelperFunctions::log(const char* s, int n){
    std::ofstream ofs;
    ofs.open ("server.log", std::ofstream::out | std::ofstream::app);
    ofs.write(s,n);

    ofs.close();
}
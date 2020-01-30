#include <argon2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <list>
#include <time.h>
#include <fstream> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "PasswdMgr.h"
#include "FileDesc.h"
#include "strfuncts.h"

const int hashlen = 32;
const int saltlen = 16;

PasswdMgr::PasswdMgr(const char *pwd_file):_pwd_file(pwd_file) {

}


PasswdMgr::~PasswdMgr() {

}

/*******************************************************************************************
 * checkUser - Checks the password file to see if the given user is listed
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkUser(const char *name) {
   std::vector<uint8_t> passwd, salt;

   bool result = findUser(name, passwd, salt);

   return result;
     
}

/*******************************************************************************************
 * checkPasswd - Checks the password for a given user to see if it matches the password
 *               in the passwd file
 *
 *    Params:  name - username string to check (case insensitive)
 *             passwd - password string to hash and compare (case sensitive)
 *    
 *    Returns: true if correct password was given, false otherwise
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkPasswd(const char *name, const char *passwd) {
   std::vector<uint8_t> userhash; // hash from the password file
   std::vector<uint8_t> passhash; // hash derived from the parameter passwd
   std::vector<uint8_t> salt;

   // Check if the user exists and get the passwd string
   if (!findUser(name, userhash, salt)){
      return false;
   }

   hashArgon2(passhash, salt, passwd, &salt);

   if (userhash == passhash)
      return true;

   return false;
}

/*******************************************************************************************
 * changePasswd - Changes the password for the given user to the password string given
 *
 *    Params:  name - username string to change (case insensitive)
 *             passwd - the new password (case sensitive)
 *
 *    Returns: true if successful, false if the user was not found
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            writing
 *
 *******************************************************************************************/

bool PasswdMgr::changePasswd(const char *name, const char *passwd) {
   /* std::fstream file;
   file.open("newPasswd", std::fstream::out);

   this->addUser(name, passwd); */
   std::cout << __LINE__ << "\n";
   int results = 0;
   std::string newLine = "\n";
   std::vector<uint8_t> password, salt;
   if (!findUser(name, password, salt)){
      return false;
   }

   std::cout << __LINE__ << "\n";
   //FileFD pwfile(_pwd_file.c_str());
   int pwfile = open(_pwd_file.c_str(), O_RDWR);
   // if (!pwfile.openFile(FileFD::writefd))
   //    throw pwfile_error("Could not open passwd file for writing");

   // Password file should be in the format username\n{32 byte hash}{16 byte salt}\n
   bool eof = false;
   while (!eof) {
      std::string uname = "";

      char nextChar[1];
      memset(nextChar, '0' , 1);
   std::cout << __LINE__ << "\n";
      //get user name first
      while(nextChar[0] != '\n'){
         std::cout << __LINE__ << "\n";
         int retVal = read(pwfile, nextChar, 1);
         if(nextChar[0] != '\n'){ uname += nextChar[0]; }
         std::cout << nextChar << "\n";
         if(retVal < 1){ 
            eof = true;
            break;
         }
      }
      memset(nextChar, '0' , 1);
      std::cout << uname   << "\n";
std::cout << __LINE__ << "\n";
      if (uname.compare(name) == 0) {
         std::vector<uint8_t> hash;
         std::vector<uint8_t> maybeRetSalt;
         
         this->hashArgon2(hash, maybeRetSalt, passwd, &salt);

         //write hash and then salt followed by newline
         char hashBuf[64];
         for(int i = 0; i < 32; i++){
            hashBuf[i] = hash.at(i);
         }
         results += write(pwfile, hashBuf, 32);
         //salt
         char saltBuf[64];
         for(int i = 0; i < 16; i++){
            saltBuf[i] = salt.at(i);
         }
         results += write(pwfile, saltBuf, 16);
         results += write(pwfile, newLine.c_str() , 1);
         close(pwfile);
         return true;
      }
std::cout << __LINE__ << "\n";
      //get hash
      for(int i = 0; i < 32; i++){
         int retVal = read(pwfile, nextChar, 1);
         if(retVal < 1){ 
            eof = true;
            break;
          }
      }
std::cout << __LINE__ << "\n";

      //get salt
      for(int i = 0; i < 16; i++){
         int retVal = read(pwfile, nextChar, 1);
         if(retVal < 1){ 
            eof = true;
            break;
          }
      }
      std::cout << __LINE__ << "\n";

      //read the final newline
      int retVal = read(pwfile, nextChar, 1);
      memset(nextChar, '0' , 1);
   }

   std::cout << __LINE__ << "\n";

   return true;
}

/*****************************************************************************************************
 * readUser - Taking in an opened File Descriptor of the password file, reads in a user entry and
 *            loads the passed in variables
 *
 *    Params:  pwfile - FileDesc of password file already opened for reading
 *             name - std string to store the name read in
 *             hash, salt - vectors to store the read-in hash and salt respectively
 *
 *    Returns: true if a new entry was read, false if eof reached 
 * 
 *    Throws: pwfile_error exception if the file appeared corrupted
 *
 *****************************************************************************************************/

bool PasswdMgr::readUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   // Insert your perfect code here!
   char nextChar[1];
   memset(nextChar, '0' , 1);
   hash.clear();
   salt.clear();

   //get user name first
   while(nextChar[0] != '\n'){
      int retVal = read(pwfile.getFD(), nextChar, 1);
      if(retVal < 1){ return false; }
      if(nextChar[0] != '\n'){ name += nextChar; }
   }
   memset(nextChar, '0' , 1);

   //get hash
   for(int i = 0; i < 32; i++){
      int retVal = read(pwfile.getFD(), nextChar, 1);
      if(retVal < 1){ return false; }
      hash.push_back(nextChar[0]); 
   }
   memset(nextChar, '0' , 1);

   //get salt
   for(int i = 0; i < 16; i++){
      int retVal = read(pwfile.getFD(), nextChar, 1);
      if(retVal < 1){ return false; }
      salt.push_back(nextChar[0]);
   }
   //read the final newline
   int retVal = read(pwfile.getFD(), nextChar, 1);
   memset(nextChar, '0' , 1);

   return true;
}

/*****************************************************************************************************
 * writeUser - Taking in an opened File Descriptor of the password file, writes a user entry to disk
 *
 *    Params:  pwfile - FileDesc of password file already opened for writing
 *             name - std string of the name 
 *             hash, salt - vectors of the hash and salt to write to disk
 *
 *    Returns: bytes written
 *
 *    Throws: pwfile_error exception if the writes fail
 *
 *****************************************************************************************************/

int PasswdMgr::writeUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   int results = 0;
   std::string newLine = "\n";

   //write uname followed by newline
   results += write(pwfile.getFD(), name.c_str(), name.length()); 
   results += write(pwfile.getFD(), newLine.c_str() , 1);

   //write hash and then salt followed by newline
   char hashBuf[64];
   for(int i = 0; i < 32; i++){
      hashBuf[i] = hash.at(i);
   }
   results += write(pwfile.getFD(), hashBuf, 32);
   //salt
   char saltBuf[64];
   for(int i = 0; i < 16; i++){
      saltBuf[i] = salt.at(i);
   }
   results += write(pwfile.getFD(), saltBuf, 16);
   results += write(pwfile.getFD(), newLine.c_str() , 1);

   return results; 
}

/*****************************************************************************************************
 * findUser - Reads in the password file, finding the user (if they exist) and populating the two
 *            passed in vectors with their hash and salt
 *
 *    Params:  name - the username to search for
 *             hash - vector to store the user's password hash
 *             salt - vector to store the user's salt string
 *
 *    Returns: true if found, false if not
 *
 *    Throws: pwfile_error exception if the pwfile could not be opened for reading
 *
 *****************************************************************************************************/

bool PasswdMgr::findUser(const char *name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt) {

   FileFD pwfile(_pwd_file.c_str());

   if (!pwfile.openFile(FileFD::readfd))
      throw pwfile_error("Could not open passwd file for reading");

   // Password file should be in the format username\n{32 byte hash}{16 byte salt}\n
   bool eof = false;
   while (!eof) {
      std::string uname;

      if (!readUser(pwfile, uname, hash, salt)) {
         eof = true;
         continue;
      }

      if (uname.compare(name) == 0) {
         pwfile.closeFD();
         return true;
      }
   }

   hash.clear();
   salt.clear();
   pwfile.closeFD();
   return false;
}



/*****************************************************************************************************
 * hashArgon2 - Performs a hash on the password using the Argon2 library. Implementation algorithm
 *              taken from the http://github.com/P-H-C/phc-winner-argon2 example. 
 *
 *    Params:  dest - the std string object to store the hash
 *             passwd - the password to be hashed
 *
 *    Throws: runtime_error if the salt passed in is not the right size
 *****************************************************************************************************/
void PasswdMgr::hashArgon2(std::vector<uint8_t> &ret_hash, std::vector<uint8_t> &ret_salt, 
                           const char *in_passwd, std::vector<uint8_t> *in_salt) {
   
   uint8_t hash1[32];
   uint8_t hash2[32];

   uint8_t salt[16];
   for(int i = 0; i < 16; i++){
      salt[i] = in_salt->at(i);
   }
   std::cout << __LINE__ << "\n";

   uint8_t *pwd = (uint8_t *)strdup(in_passwd);
   uint32_t pwdlen = strlen((char *)pwd);
   std::cout << __LINE__ << "\n";

   uint32_t t_cost = 2;            // 1-pass computation
   uint32_t m_cost = (1<<16);      // 64 mebibytes memory usage
   uint32_t parallelism = 1;       // number of threads and lanes
   std::cout << __LINE__ << "\n";

   // high-level API
   argon2i_hash_raw(t_cost, m_cost, parallelism, pwd, pwdlen, salt, 16, hash1, 32);
   std::cout << __LINE__ << "\n";

   std::cout << hash1 << "\n";

   for(auto i: hash1){
      ret_hash.push_back(i);
   }


}

/****************************************************************************************************
 * addUser - First, confirms the user doesn't exist. If not found, then adds the new user with a new
 *           password and salt
 *
 *    Throws: pwfile_error if issues editing the password file
 ****************************************************************************************************/

void PasswdMgr::addUser(const char *name, const char *passwd) {
   if(this->checkUser(name)){ return; }

   FileFD pwfile(_pwd_file.c_str());

   if (!pwfile.openFile(FileFD::appendfd))
      throw pwfile_error("Could not open passwd file for reading");

   srand(time(0)); 
   std::vector<uint8_t> salt;
   for(int i = 0;i < 16; i++){
      salt.push_back(rand());
   }
   std::vector<uint8_t> hash;
   std::vector<uint8_t> maybeRetSalt;
   std::string stringName(name);
   this->hashArgon2(hash, maybeRetSalt, passwd, &salt);
   this->writeUser(pwfile, stringName, hash, salt);
   pwfile.closeFD();
}


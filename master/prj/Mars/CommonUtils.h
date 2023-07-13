#include <afxsock.h>
#include <iostream>
#include <string>
//#include <openssl/sha.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>


#define _WINSOCK_DEPRECATED_NO_WARNINGS

using namespace std;

string getLocalIPAddress();
string sha256(string strIn);
CStringW stringToCStringW(string str);
string generateRandomPassword(int length);

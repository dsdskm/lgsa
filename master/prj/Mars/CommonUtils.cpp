#include "pch.h"
#include "CommonUtils.h"
#include <random>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

string getLocalIPAddress() {
    string ipAddress;

    char hostName[255];
    if (gethostname(hostName, sizeof(hostName)) == 0) {
        hostent* host = gethostbyname(hostName);
        if (host != nullptr) {
            for (int i = 0; host->h_addr_list[i] != nullptr; i++) {
                in_addr address;
                memcpy(&address, host->h_addr_list[i], sizeof(in_addr));
                ipAddress = inet_ntoa(address);
#if 1
                if (ipAddress.substr(0, 10) == "192.168.3." || ipAddress.substr(0, 10) == "192.168.4.") {
                    return ipAddress;
                }
#else
                break;
#endif
            }
        }
    }
    return ipAddress;
}


string generateRandomPassword(int length) {
    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
    std::string password;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, charset.length() - 1);

    for (int i = 0; i < length; ++i) {
        password += charset[distribution(generator)];
    }

    return password;
}

/*
string sha256(string strIn)
{
    unsigned char digest[SHA256_DIGEST_LENGTH];
    char chIn[128];
    strcpy(chIn, strIn.c_str());

    SHA256((unsigned char*)&chIn, strlen(chIn), (unsigned char*)&digest);

    char mdString[SHA256_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);

    string strOut(mdString);

    return strOut;
}
*/

CStringW stringToCStringW(string str) {
    CStringW cstrw(str.c_str());
    return cstrw;
}

string sha256(string strIn) {
    string hash;
    CryptoPP::SHA256 sha256;
    CryptoPP::StringSource(strIn, true,
        new CryptoPP::HashFilter(sha256,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(hash), false)));

    return hash;
}
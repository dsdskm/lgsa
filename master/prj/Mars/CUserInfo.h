#pragma once

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "CConferenceInfo.h"

using namespace std;
using json = nlohmann::json;

class CUserInfo {
private:
    string id;
    string email;
    string password;
    string firstName;
    string lastName;
    string ipAddress;
    string status;
    list<CUserInfo> buddy;
    list<CConferenceInfo> conferenceList;

public:
    CUserInfo(string id);
    CUserInfo(string id, string email, string firstName, string lastName, string ipAddress, string status);
    void printInfo();
    json sendGet(string path);
    json send(string method, string path, json body);
    void getUserInfo();

    string getEmail();
    string getFirstName();
    string getLastName();
    string getFullName();
    string getIpAddress();
    string getId();
    string getStatus();
    list<CUserInfo> getBuddy();
    list<CConferenceInfo> getConferenceList();

    void setEmail(string _email);
    void setFirstName(string _firstName);
    void setLastName(string _lastName);
    void setIpAddress(string _ipAddress);
    void setStatus(string _status);

    void update();
};
#pragma once

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "CConferenceInfo.h"
#include "CUserInfo.h"

using namespace std;
using json = nlohmann::json;

class CParticipant {
private:
    int idx;
    string id;
    string email;
    string firstName;
    string lastName;
    string ipAddress;
    bool activate;
 
public:
    CParticipant(int idx, string id, string email, string firstName, string lastName, string ipAddress, bool activate);
    CParticipant(const CParticipant& cp);
    CParticipant();

    int getIdx();
    string getId();
    string getEmail();
    string getFirstName();
    string getLastName();
    string getFullName();
    string getIpAddress();
    bool getActivateState();
    void setActivate(bool activate);

};
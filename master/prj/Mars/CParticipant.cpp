#include "CParticipant.h"
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <nlohmann/json.hpp>
#include "CRest.h"
#include "CMarsFunctions.h"
#include "CConferenceInfo.h"

using namespace std;
using json = nlohmann::json;

// Constructor
CParticipant::CParticipant(int _idx, string _id, string _email, string _firstName, string _lastName, string _ipAddress, bool _activate)
    : idx(_idx), id(_id), email(_email), firstName(_firstName), lastName(_lastName), ipAddress(_ipAddress), activate(_activate) {}

CParticipant::CParticipant(const CParticipant& cp) {
    idx = cp.idx;
    id = cp.id;
    email = cp.email;
    firstName = cp.firstName;
    lastName = cp.lastName;
    ipAddress = cp.ipAddress;
    activate = cp.activate;
}

CParticipant::CParticipant() {}

// Member function

// Getter and setter methods

int CParticipant::getIdx() {
    return idx;
}

string CParticipant::getEmail() {
    return email;
}

string CParticipant::getFirstName() {
    return firstName;
}

string CParticipant::getLastName() {
    return lastName;
}

string CParticipant::getFullName() {
    return firstName + " " + lastName;
}

string CParticipant::getIpAddress() {
    return ipAddress;
}

string CParticipant::getId() {
    return id;
}

bool CParticipant::getActivateState() {
    return activate;
}

void CParticipant::setActivate(bool _activate) {
    activate = _activate;
}
#include "pch.h"
#include "CUserInfo.h"
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
CUserInfo::CUserInfo(string _id) : id(_id) {}
CUserInfo::CUserInfo(string _id, string _email, string _firstName, string _lastName, string _ipAddress, string _status)
    : id(_id), email(_email), firstName(_firstName), lastName(_lastName), ipAddress(_ipAddress), status(_status) {}


// Member function
void CUserInfo::printInfo() {
    std::cout << "url: " << id << std::endl;
    std::cout << "str: " << password << std::endl;
}

// Getter and setter methods
//json CUserInfo::sendGet(string path) {

void CUserInfo::getUserInfo() {
/*
    CRest rest;
    json body;
    json res = rest.send("POST", "user" + id, body);
    email = res["ret"]["email"];
    password = res["ret"]["password"];
    firstName = res["ret"]["firstname"];
    lastName = res["ret"]["lastname"];
    ipAddress = res["ret"]["ipAddress"];
*/
    this->update();
//    CMarsFunctions mMars;
//    CUserInfo mUserInfo = mMars.getUserInfo(id);
//    email = mUserInfo.email;
//    firstName = mUserInfo.firstName;
//    lastName = mUserInfo.lastName;
//    ipAddress = mUserInfo.ipAddress;
}


string CUserInfo::getEmail() {
    return email;
}

string CUserInfo::getFirstName() {
    return firstName;
}

string CUserInfo::getLastName() {
    return lastName;
}

string CUserInfo::getFullName() {
    return firstName + " " + lastName;
}

string CUserInfo::getIpAddress() {
    return ipAddress;
}

string CUserInfo::getStatus() {
    return status;
}

string CUserInfo::getId() {
    return id;
}

list<CUserInfo> CUserInfo::getBuddy() {
    return buddy;
}

list<CConferenceInfo> CUserInfo::getConferenceList() {
    return conferenceList;
}

void CUserInfo::setEmail(string _email) {
    email = _email;
}

void CUserInfo::setFirstName(string _firstName) {
    firstName = _firstName;
}

void CUserInfo::setLastName(string _lastName) {
    lastName = _lastName;
}

void CUserInfo::setIpAddress(string _ipAddress) {
    ipAddress = _ipAddress;
}

void CUserInfo::setStatus(string _status) {
    status = _status;
}

void CUserInfo::update() {
    CMarsFunctions mMars;
    CUserInfo mUserInfo = mMars.getUserInfo(id);
    email = mUserInfo.email;
    firstName = mUserInfo.firstName;
    lastName = mUserInfo.lastName;
    ipAddress = mUserInfo.ipAddress;
    status = mUserInfo.status;

//    json res = mMars.getContactList(id);
    buddy.clear();
    for (auto p : mMars.getContactList(id)) {
        buddy.push_back(CUserInfo(p["id"], p["email"], p["firstname"], p["lastname"], p["ipAddress"], p["status"]));
    }

//    res = mMars.getConferenceList(id);
    conferenceList.clear();
    for (auto p : mMars.getConferenceList(id)) {
        if (p["joinUsers"].is_null()) {
//            AfxMessageBox(_T("kkk"));
            p["joinUsers"] = json::array();
        }
        conferenceList.push_back(CConferenceInfo(p["id"], p["topic"], p["start"], p["duration"], p["participant"], p["joinUsers"]));
    }
}
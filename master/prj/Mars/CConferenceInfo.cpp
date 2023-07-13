#include "pch.h"
#include "CConferenceInfo.h"
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <nlohmann/json.hpp>
#include "CMarsFunctions.h"

using namespace std;
using json = nlohmann::json;

// Constructor
CConferenceInfo::CConferenceInfo(string _id, string _topic, string _start, string _duration)
    : id(_id), topic(_topic), start(_start), duration(_duration) {}

CConferenceInfo::CConferenceInfo(string _id, string _topic, string _start, string _duration, list<string> _participant, list<string> _joinUsers)
    : id(_id), topic(_topic), start(_start), duration(_duration), participant(_participant), joinUsers(_joinUsers) {}
/*
CConferenceInfo::CConferenceInfo(json body) {
    id = body["id"];
    topic = body["topic"];
    start = body["start"];
    duration = body["duration"];
    participant = body["participant"];
    joinUsers = body["joinUsers"];
}
*/
// Member function


// Getter and setter methods

string CConferenceInfo::getId() {
    return id;
}

string CConferenceInfo::getTopic() {
    return topic;
}

string CConferenceInfo::getStart() {
    return start;
}

string CConferenceInfo::getDuration() {
    return duration;
}

list<string> CConferenceInfo::getParticipant() {
    return participant;
}

list<string> CConferenceInfo::getJoinUsers() {
    return joinUsers;
}
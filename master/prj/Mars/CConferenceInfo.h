#pragma once

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class CConferenceInfo
{
private:
    string id;
    string topic;
    string start;
    string duration;
    list<string> participant;
    list<string> joinUsers;

public:
    CConferenceInfo(string id, string topic, string start, string duration);
    CConferenceInfo(string id, string topic, string start, string duration, list<string> participant, list<string> joinUsers);
    CConferenceInfo(json body);
    void getConferenceInfo();

    string getId();
    string getTopic();
    string getStart();
    string getDuration();
    list<string> getParticipant();
    list<string> getJoinUsers();

    void update();
};


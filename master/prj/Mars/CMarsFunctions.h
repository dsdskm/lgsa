#pragma once
#include "CRest.h"
#include "CUserInfo.h"

class CMarsFunctions : public CRest
{
public:
	CMarsFunctions();
	bool registerUser(string id, string email, string password, string firstName, string lastName);
	bool changePassword(string id, string password);
	bool changeEmail(string id, string email);
	bool resetPassword(string id, string email);
	bool authenticateUser(string id, string password);
	json getContactList(string id);
	json getConference(string id);
	json getConferenceList(string id);
	CUserInfo getUserInfo(string id);
	json searchUser(string id, string keyword);
	bool createContact(string id, list<string> users);
	json updateContact(string id, list<string> users);
	string createConference(string topic, string start_time, list<string> participant, string creator);
	bool updateJoinConference(string id, string conferenceId, bool isJoin);
	bool updateConference(string conferenceId, string topic, string startTime, list<string> participant);
	json getAllUserList();
	bool updateUserEnable(string userId, bool enable);
	bool requestCall(string conferenceId, string creatorId);
};
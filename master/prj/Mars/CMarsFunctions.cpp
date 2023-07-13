#include "pch.h"
#include "CMarsFunctions.h"
#include "CommonUtils.h"
#include "CUserInfo.h"


CMarsFunctions::CMarsFunctions() {
	setUrl(getUrlFromText());
}


bool CMarsFunctions::registerUser(string id, string email, string password, string firstName, string lastName) {
	json body;
	body["id"] = id;
	body["email"] = email;
	body["password"] = password;
	body["firstname"] = firstName;
	body["lastname"] = lastName;
	body["ipAddress"] = getLocalIPAddress();

	json res = send("POST", "user", body);
	if (res["ret"] != "201") {
		return false;
	}
	return true;
}

bool CMarsFunctions::changePassword(string id, string password) {
	json body;
	body["password"] = password;

	json res = send("PUT", "user/" + id + "/password", body);
	return true;
}

bool CMarsFunctions::changeEmail(string id, string email) {
	json body;
	body["email"] = email;

	json res = send("PUT", "user/" + id + "/email", body);
	return true;
}

bool CMarsFunctions::resetPassword(string id, string email) {
	json body;
	string pw = generateRandomPassword(12);
	body["email"] = email;
	body["encryptedPassword"] = sha256(pw);
	body["decryptedPassword"] = pw;

	json res = send("DELETE", "user/" + id + "/password", body);
	return true;
}

bool CMarsFunctions::authenticateUser(string id, string password) {
	json body;
	body["id"] = id;
	body["password"] = password;
	body["ipAddress"] = getLocalIPAddress();

	json res = send("PUT", "user/" + id, body);

	if (res["ret"] == "403") {
		return false;
	}

	return true;
}

json CMarsFunctions::getContactList(string id) {
	json res = send("GET", "contact/" + id + "/list", NULL);

	return res["ret"];
}

json CMarsFunctions::getConferenceList(string id) {
	json res = send("GET", "conference/" + id + "/list", NULL);

	return res["ret"];
}

json CMarsFunctions::getConference(string id) {
	json res = send("GET", "conference/" + id, NULL);

	return res["ret"];
}


CUserInfo CMarsFunctions::getUserInfo(string id) {
	json res = send("GET", "user/" + id, NULL);
	CUserInfo mUserInfo(id);

	mUserInfo.setEmail(res["ret"]["email"]);
	mUserInfo.setFirstName(res["ret"]["firstname"]);
	mUserInfo.setLastName(res["ret"]["lastname"]);
	mUserInfo.setIpAddress(res["ret"]["ipAddress"]);
	mUserInfo.setStatus(res["ret"]["status"]);

	return mUserInfo;
}

json CMarsFunctions::searchUser(string id, string keyword) {
	json res = send("GET", "user/" + id + "/search?key=" + keyword, NULL);
	return res["ret"];
}

bool CMarsFunctions::createContact(string id, list<string> users) {
	json body;
	body["id"] = id;
	body["users"] = users;

	json res = send("POST", "contact", body);
	if (res["ret"] != "201") {
		return false;
	}
	return true;
}

json CMarsFunctions::updateContact(string id, list<string> users) {
	json body;
	body["id"] = id;
	body["users"] = users;

	json res = send("PUT", "contact/" + id + "/update", body);
	return res["ret"];
}


string CMarsFunctions::createConference(string topic, string start_time, list<string> participant, string creator) {
	json body;
	body["topic"] = topic;
	body["start"] = start_time;
	body["participant"] = participant;
	body["creator"] = creator;

	json res = send("POST", "conference", body);

	return res["ret"];
}

bool CMarsFunctions::updateJoinConference(string id, string conferenceId, bool isJoin) {
	json body;
	body["userId"] = id;
	body["isJoin"] = isJoin;

	json res = send("PUT", "conference/" + conferenceId + "/join", body);
	if (res["ret"] == "200") {
		return true;
	}
	return false;
}

bool CMarsFunctions::updateConference(string conferenceId, string topic, string startTime, list<string> participant) {
	json body;
	body["topic"] = topic;
	body["start"] = startTime;
	body["duration"] = "undefined";
	body["participant"] = participant;

	json res = send("PUT", "conference/" + conferenceId + "/update", body);
	if (res["ret"] == "200") {
		return true;
	}
	return false;
}


json CMarsFunctions::getAllUserList() {
	json res = send("GET", "user/0/list", NULL);
	return res["ret"];
}

bool CMarsFunctions::updateUserEnable(string userId, bool enable) {
	json body;
	body["enable"] = enable;
	json res = send("PUT", "user/" + userId + "/enable", body);
	if (res["ret"] == "200") {
		return true;
	}
	return false;
}
bool CMarsFunctions::requestCall(string conferenceId, string creatorId) {
	json body;
	body["userId"] = creatorId;

	json res = send("POST", "conference/" + conferenceId + "/callRequest", body);

	if (res["ret"] == "200") {
		return true;
	}
	return false;
}

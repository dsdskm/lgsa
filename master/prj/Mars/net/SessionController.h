#ifndef MARS_SESSION_CONTROLLER_H
#define MARS_SESSION_CONTROLLER_H

#include <nlohmann/json.hpp>

namespace net {

class SessionEventHandler {
 public:
  virtual void OnCallReservation(
      const std::string& call_id, std::list<std::string> participant_id_list) = 0;
  virtual void OnCallRequest(const std::string& call_id) = 0;
  virtual void OnCallJoined(const std::string& call_id, const std::string& joined_uid) = 0;
  virtual void OnCallLeft(const std::string& call_id, const std::string& left_uid) = 0;
  virtual void OnCallReject(const std::string& call_id, const std::string& rejected_uid) = 0;
  virtual void OnUserUpdate(const std::string& updated_uid) = 0;
};

class SessionController {
 public:
  // returns singleton
  static SessionController* Get();

  virtual void Shutdown() = 0;

  virtual void AddHandler(SessionEventHandler*) = 0;
  virtual void RemoveHandler(SessionEventHandler*) = 0;
  /*
  virtual bool RegisterUser(
      const std::string& user_id, const std::string& user_password,
      const std::string& first_name, const std::string& last_name) = 0;

  virtual bool ChangePassword(
      const std::string& user_id, const std::string& user_password) = 0;

  virtual bool ResetPassword(const std::string& user_id, const std::string& email) = 0;
  virtual bool AuthenticateUser(const std::string& user_id, const std::string& password) = 0;
  virtual nlohmann::json GetContactList(const std::string& user_id) = 0;
  virtual nlohmann::json GetConferenceList(const std::string& user_id) = 0;
  */
};
  
}

#endif // MARS_SESSION_CONTROLLER_H

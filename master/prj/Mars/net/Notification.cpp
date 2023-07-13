#include "Notification.h"
#include <map>
#include <nlohmann/json.hpp>
#include <MarsLog.h>
#include <iostream>
#include <sstream>

namespace net {

static Notification::NotificationType get_notification_type(const std::string& type_str) {
  static std::map<std::string, Notification::NotificationType> type_map;
  if (type_map.empty()) {
    type_map.insert(std::make_pair("ping", Notification::NOTI_PING));
    type_map.insert(std::make_pair("callReserve", Notification::NOTI_CALL_RESERVE));
    type_map.insert(std::make_pair("callRequest", Notification::NOTI_CALL_REQUEST));
    type_map.insert(std::make_pair("callJoined", Notification::NOTI_CALL_JOINED));
    type_map.insert(std::make_pair("callLeft", Notification::NOTI_CALL_LEFT));
    type_map.insert(std::make_pair("callReject", Notification::NOTI_CALL_REJECT));
    type_map.insert(std::make_pair("userUpdate", Notification::NOTI_USER_UPDATE));
  }

  auto it = type_map.find(type_str);
  if (it == type_map.end())
    return Notification::NOTI_UNKNOWN;
  return it->second;
}

Notification Notification::FromJson(const std::string& json) {
  Notification noti;
  nlohmann::json parsed = nlohmann::json::parse(json);
  noti.type_ = get_notification_type(parsed["type"]);
  switch (noti.type_) {
    case NOTI_PING:
      noti.trial_ = parsed["trial"];
      break;
    case NOTI_CALL_RESERVE: {
      noti.cid_ = parsed["cid"];
      nlohmann::json p = parsed["participant"];
      for (auto it = p.begin(); it != p.end(); it++)
        noti.participant_.push_back(it.value());
      break;
    }
    case NOTI_CALL_JOINED:
    case NOTI_CALL_LEFT:
    case NOTI_CALL_REJECT:
      noti.participant_.push_back(parsed["uid"]);
      // INTENTIONAL NO BREAK
    case NOTI_CALL_REQUEST:
      noti.cid_ = parsed["cid"];
      break;
    case NOTI_USER_UPDATE:
      noti.participant_.push_back(parsed["uid"]);
      break;
    default:
      NLOG_E("Unknown Notification type : %d\n", noti.type_);
      ;
  }
  return noti;
}

std::string Notification::dump() {
  std::ostringstream oss;
  oss << "type: " << type() << ", trial: " << trial() << ", cid: " << cid();
  return oss.str();
}

std::string Pong::ToJson() {
  nlohmann::json json;
  json["type"] = "echo";
  json["trial"] = trial_;
  return json.dump();
}

bool NotificationReader::Feed(char* in, size_t len) {
  size_t tail = in_buffer_.size();
  in_buffer_.resize(tail + len);
  memcpy(in_buffer_.data() + tail, in, len);
  char buf[1024];
  while (true) {
    std::string json = Split();
    if (json.empty())
      break;
    Notification noti = Notification::FromJson(json);
    if (noti.type() != Notification::NOTI_UNKNOWN)
      notifications_.push(noti);
  }
  return false;
}

bool NotificationReader::HasNextNotification() {
  return !notifications_.empty();
}

Notification NotificationReader::PopNotification() {
  Notification noti = notifications_.front();
  notifications_.pop();
  return noti;
}


std::string NotificationReader::Split() {
  size_t len = strlen(in_buffer_.data());
  // std::cout << "feafeafeaf: " << len << " inbuf: " << in_buffer_.size() << std::endl;
  if (len > in_buffer_.size())
    return std::string();
  std::string ret(in_buffer_.data(), len);
  // std::cout << ret.c_str() << std::endl;
  size_t left = in_buffer_.size() - len - 1;
  if (left > 0) {
    std::vector<char> tmp(left);
    // std::cout << "afeafea: " << (in_buffer_.data() + len + 1) << std::endl;
    memcpy(tmp.data(), in_buffer_.data() + len + 1, left);
    // std::cout << tmp.data() << " len: " << tmp.size();
    in_buffer_.swap(tmp);
    return ret;
  }
  in_buffer_.resize(0);
  return ret;
}

} // namespace net

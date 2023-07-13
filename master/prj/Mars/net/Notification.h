#ifndef MARS_NOTIFICATION_H
#define MARS_NOTIFICATION_H

#include <string>
#include <queue>
#include <list>

namespace net {

class Notification {
 public:
  enum NotificationType {
    NOTI_UNKNOWN,
    NOTI_PING,
    NOTI_CALL_RESERVE,
    NOTI_CALL_REQUEST,
    NOTI_CALL_JOINED,
    NOTI_CALL_LEFT,
    NOTI_CALL_REJECT,
    NOTI_USER_UPDATE,
  };
  Notification() : type_(NOTI_UNKNOWN) {}
  NotificationType type() const { return type_; }
  std::string cid() const { return cid_; }
  std::string trial() const { return trial_; }
  std::list<std::string> participant() { return participant_; }
  static Notification FromJson(const std::string& json);

  std::string dump();
 private:
  NotificationType type_;
  std::string cid_;
  std::string trial_;
  std::list<std::string> participant_;
  std::string dump_;
};

class Pong {
 public:
  Pong(const std::string& trial) : trial_(trial) {}
  std::string ToJson();
 private:
  std::string trial_;
};

class NotificationReader {
 public:
  bool Feed(char* in, size_t len);
  bool HasNextNotification();
  Notification PopNotification();
 private:
  std::string Split();
  std::vector<char> in_buffer_;
  std::queue<Notification> notifications_;
};
  
} // namespace net

#endif // MARS_NOTIFICATION_H

#ifndef MARS_SESSION_CONTROLLER_IMPL_H
#define MARS_SESSION_CONTROLLER_IMPL_H

#include "NetCommon.h"
#include "Notification.h"
#include <mutex>
#include <queue>
#include <set>

namespace net {
  
class SessionControllerImpl : public SessionController {
 public:
  enum EVENT_TYPE {
    EVENT_EXIT = 0,
    EVENT_ACCEPT,
    EVENT_SOCKET_IO,
    EVENT_TYPE_COUNT
  };
  SessionControllerImpl();

  void Start();
  void Shutdown() override;
  
  void AddHandler(SessionEventHandler*) override;
  void RemoveHandler(SessionEventHandler*) override;

  static DWORD Runner(void*);
private:
  void NotifyHandlers(Notification&);
  void Run();

  SOCKET listen_socket_;
  SOCKET notification_socket_;
  HANDLE thread_;
  HANDLE events_[EVENT_TYPE_COUNT];
  bool writable_;
  std::set<SessionEventHandler*> handlers_;
  std::recursive_mutex mutex_;
  NotificationReader reader_;
  std::queue<std::string> ping_;
};


} // namespace net

#endif // MARS_SESSION_CONTROLLER_IMPL_H

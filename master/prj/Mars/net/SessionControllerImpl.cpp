#include "NetCommon.h"
#include "SessionController.h"
#include "SessionControllerImpl.h"
#include <nlohmann/json.hpp>
#include <MarsLog.h>

#pragma comment(lib, "Ws2_32.lib")

#define READ_BUFFER_SIZE 4096

namespace net {

SessionController* SessionController::Get() {
  static SessionController* instance = NULL;
  if (!instance) {
    instance = new SessionControllerImpl();
    ((SessionControllerImpl*) instance)->Start();
  }
  return instance;
}

SessionControllerImpl::SessionControllerImpl()
    : listen_socket_(INVALID_SOCKET),
      notification_socket_(INVALID_SOCKET),
      thread_(INVALID_HANDLE_VALUE),
      events_{INVALID_HANDLE_VALUE, },
      writable_(false) {
}
      

void SessionControllerImpl::Start() {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
    printf("Failed. Error Code : %d", WSAGetLastError());
    return;
  }
  thread_ = CreateThread(NULL, 0, &SessionControllerImpl::Runner, this, 0, NULL);
}


void SessionControllerImpl::Shutdown() {
  SetEvent(events_[EVENT_EXIT]);
  WaitForSingleObject(thread_, INFINITE);
  WSACleanup();
}

void SessionControllerImpl::AddHandler(SessionEventHandler* handler) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  handlers_.insert(handler);
}

void SessionControllerImpl::RemoveHandler(SessionEventHandler* handler) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  handlers_.erase(handler);
}

DWORD SessionControllerImpl::Runner(void* that) {
  SessionControllerImpl* self = (SessionControllerImpl*)that;
  self->Run();
  return 0;
}

void SessionControllerImpl::NotifyHandlers(Notification& noti) {
  for (auto& handler : handlers_) {
    switch (noti.type()) {
      case Notification::NOTI_CALL_RESERVE:
        handler->OnCallReservation(noti.cid(), noti.participant());
        break;
      case Notification::NOTI_CALL_REQUEST:
        handler->OnCallRequest(noti.cid());
        break;
      case Notification::NOTI_CALL_JOINED:
        handler->OnCallJoined(noti.cid(), noti.participant().front());
        break;
      case Notification::NOTI_CALL_LEFT:
        handler->OnCallLeft(noti.cid(), noti.participant().front());
        break;
      case Notification::NOTI_CALL_REJECT:
        handler->OnCallReject(noti.cid(), noti.participant().front());
        break;
      case Notification::NOTI_USER_UPDATE:
        handler->OnUserUpdate(noti.participant().front());
        break;
      default:
        NLOG_E("Unknown Notification type\n");;
        return;
    }
  }
}
  
void SessionControllerImpl::Run() {
  if ((listen_socket_ = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    NLOG_E("Unable to init socket for Server Notification: %d\n", WSAGetLastError());
    return;
  }

  // initialize events
  events_[EVENT_EXIT] = CreateEvent(NULL, FALSE, FALSE, NULL);
  events_[EVENT_ACCEPT] = WSACreateEvent();
  WSAEventSelect(listen_socket_, events_[EVENT_ACCEPT], FD_ACCEPT);

  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SESSION_PORT);

  if (bind(listen_socket_, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    NLOG_E("Unable to bind notification server : %d", WSAGetLastError());
    return;
  }
  
  if (listen(listen_socket_, 5)) {
    NLOG_E("listen failed with error : %d\n", WSAGetLastError());
    return;
  }
  
  char readbuf[READ_BUFFER_SIZE];
  while (true) {
    int event_count = notification_socket_ == INVALID_SOCKET ? EVENT_ACCEPT + 1 : EVENT_TYPE_COUNT;
    DWORD dwEvent = WaitForMultipleObjects(event_count, events_, FALSE, INFINITE);
    dwEvent -= WAIT_OBJECT_0;
    switch (dwEvent) {
      case EVENT_EXIT:
        return;
      case EVENT_ACCEPT: {
        // Accept connection request
        NLOG_D("accept event...\n");
        WSANETWORKEVENTS network_events;
        if (WSAEnumNetworkEvents(listen_socket_, events_[EVENT_ACCEPT], &network_events) == SOCKET_ERROR) {
          NLOG_E("WSAEnumNetworkEvents failed with : %d\n", WSAGetLastError());
          continue;
        }

        if ((network_events.lNetworkEvents & FD_ACCEPT) && !network_events.iErrorCode[FD_ACCEPT_BIT]) {
          NLOG_D("Accepting connection...\n");
          struct sockaddr_storage sa;
          socklen_t sa_len = sizeof(sa);
          SOCKET s = accept(listen_socket_, (struct sockaddr*)&sa, &sa_len);
          HANDLE ev = WSACreateEvent();
          WSAEventSelect(s, ev, FD_READ | FD_WRITE | FD_CLOSE);

          // std::unique_lock<std::recursive_mutex> lock(mutex_);
          // discard old connection
          if (notification_socket_ != INVALID_SOCKET) {
            // we don't need redundant server connection
            ResetEvent(events_[EVENT_SOCKET_IO]);
            CloseHandle(events_[EVENT_SOCKET_IO]);
            closesocket(notification_socket_);
          }
          notification_socket_ = s;
          events_[EVENT_SOCKET_IO] = ev;
          continue;
        }
        break;
      }
      case EVENT_SOCKET_IO: {
        NLOG_D("socket io event...\n");
        WSANETWORKEVENTS network_events;
        if (WSAEnumNetworkEvents(
                notification_socket_, events_[EVENT_SOCKET_IO], &network_events) == SOCKET_ERROR) {
          NLOG_E("WSAEnumNetworkEvents: %d\n", WSAGetLastError());
          network_events.lNetworkEvents = 0;
          continue;
        }

        // write
        if (network_events.lNetworkEvents & FD_WRITE) {
          if (network_events.iErrorCode[FD_WRITE_BIT]) {
            NLOG_E("FD_WRITE failed with error : %d\n", WSAGetLastError());
            continue;
          }
          writable_ = true;
        }

        // read
        if (network_events.lNetworkEvents & FD_READ) {
          if (network_events.iErrorCode[FD_READ_BIT]) {
            NLOG_E("FD_READ failed with error: %d\n", WSAGetLastError());
            continue;
          }

          // std::unique_lock<std::recursive_mutex> lock(mutex_);
          int r = recv(notification_socket_, readbuf, sizeof(readbuf), 0);
          NLOG_D("recv returns : %d %s\n", r, readbuf);
          
          if (r == SOCKET_ERROR) {
            int err = WSAGetLastError();
            // if (err == WSAEWOULDBLOCK) {
            //   ResetEvent(events_[EVENT_SOCKET_IO]);
            //   continue;
            // }
            NLOG_E("recv error : %d\n", err);
            continue;
          }
          reader_.Feed(readbuf, r);
        }

        {
          std::unique_lock<std::recursive_mutex> lock(mutex_);

          // Check if there're completed notifications
          while (reader_.HasNextNotification()) {
            Notification noti = reader_.PopNotification();
            if (noti.type() == Notification::NOTI_PING) {
                ping_.push(noti.trial());
                continue;
            }

            NotifyHandlers(noti);
          }
        }

        if (writable_) {
          while (!ping_.empty()) {
            std::string pong = Pong(ping_.front()).ToJson();
            int r = send(notification_socket_, pong.c_str(), pong.size(), 0);
            if (r == SOCKET_ERROR) {
              NLOG_E("Unable to send pong to server : %d\n", WSAGetLastError());
              break;
            }
            ping_.pop();
          }
        }
      }
      default:
        ;
    }
  }
}

} // namespace net

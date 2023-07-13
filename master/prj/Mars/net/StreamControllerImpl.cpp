#include "NetCommon.h"
#include "StreamController.h"
#include "StreamControllerImpl.h"
#include "StreamBuffer.h"
#include <iostream>
#include <stdio.h>
#include <MarsLog.h>

#pragma comment(lib, "Ws2_32.lib")

#define PACKET_BUFFER_SIZE 4096
#define EXPIRY_CHECK_INTERVAL 1000
#define RESEND_CHECK_INTERVAL 100
// WSAGetLastError() :
// https://learn.microsoft.com/ko-kr/windows/win32/winsock/windows-sockets-error-codes-2

namespace net {

StreamController* StreamController::Create(StreamSenderClient* sender_client) {
  return new StreamControllerImpl(sender_client);
}

StreamControllerImpl::StreamControllerImpl(StreamSenderClient* sender_client)
    : socket_(INVALID_SOCKET),
      thread_(INVALID_HANDLE_VALUE),
      timer_(INVALID_HANDLE_VALUE),
      sender_client_(sender_client) {
}

void StreamControllerImpl::Start() {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
    printf("Failed. Error Code : %d", WSAGetLastError());
    return;
  }
  thread_ = CreateThread(NULL, 0, &StreamControllerImpl::Runner, this, 0, NULL);
}

void StreamControllerImpl::Stop() {
  SetEvent(events_[EVENT_EXIT]);
  WaitForSingleObject(thread_, INFINITE);
}

void StreamControllerImpl::Shutdown() {
  Stop();
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  for (auto& p : peers_) {
    delete p.second;
  }
  peers_.clear();
  WSACleanup();
}

const int StreamControllerImpl::AddPeer(
    const std::string& ip_address, StreamReceiverClient* client) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  // remove old peer
  for (auto it = peers_.begin(); it != peers_.end(); it++) {
    if (it->second->ip_address() == ip_address) {
      peers_.erase(it);
      break;
    }
  }
  Participant* p = new Participant(ip_address, client);
  peers_.insert(std::make_pair(p->peer_id(), p));
  return p->peer_id();
}

void StreamControllerImpl::RemovePeer(const int peer_id) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  auto it = peers_.find(peer_id);
  if (it == peers_.end()) {
    std::cout << "No such peer with peer_id: " << peer_id << std::endl;
    return;
  }
  Participant* p = it->second;
  if (!p) {
    return;
  }

  if (p->peer_id() != peer_id) {
    std::cout << "given peer_id("<< peer_id
              << ") mismatches to Participant's peer_id: " << p->peer_id() << std::endl;
    return;
  }
  peers_.erase(peer_id);
  delete p;
}


int StreamControllerImpl::Send(StreamType type, char* buf, size_t buflen, bool guaranteed) {
  static int chunk_id = 0;
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  for (auto& p : peers_) {
    p.second->Send(type, buf, buflen, guaranteed);
  }
  SetEvent(events_[EVENT_SEND]);
  return ++chunk_id;
}

size_t StreamControllerImpl::Receive(
    const int peer_id, int stream_id, char* buf, size_t buflen) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  auto it = peers_.find(peer_id);
  if (it != peers_.end()) {
    return it->second->Receive(stream_id, buf, buflen);
  }
  return 0;
}

DWORD StreamControllerImpl::Runner(void* that) {
  StreamControllerImpl* self = (StreamControllerImpl*) that;
  self->Run();
  return 0;
}

void StreamControllerImpl::Run() {
  SOCKET socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket_ == INVALID_SOCKET)
    return;
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(STREAM_PORT);
  bind(socket_, (struct sockaddr*)&addr, sizeof(addr));


  events_[EVENT_RECEIVE] = WSACreateEvent();
  WSAEventSelect(socket_, events_[EVENT_RECEIVE], FD_READ);

  events_[EVENT_EXIT] = CreateEvent(NULL, FALSE, FALSE, NULL);
  events_[EVENT_SEND] = CreateEvent(NULL, FALSE, FALSE, NULL);
  events_[EVENT_EXPIRE] = CreateWaitableTimer(NULL, FALSE, NULL);
  if (events_[EVENT_EXPIRE] != NULL) {
    LARGE_INTEGER due_time;
    due_time.QuadPart = 0LL;
    if (!SetWaitableTimer(events_[EVENT_EXPIRE], &due_time, EXPIRY_CHECK_INTERVAL, NULL, NULL, 0)) {
      NLOG_I("Waitable timer init failed\n");
    }
  }

  events_[EVENT_RESEND] = CreateWaitableTimer(NULL, FALSE, NULL);
  if (events_[EVENT_RESEND] != NULL) {
    LARGE_INTEGER due_time;
    due_time.QuadPart = 0LL;
    if (!SetWaitableTimer(events_[EVENT_RESEND], &due_time, RESEND_CHECK_INTERVAL, NULL, NULL, 0)) {
      NLOG_I("Waitable timer init failed\n");
    }
  }
  
  char buf[4096];
  while (true) {
    DWORD dwEvent = WaitForMultipleObjects(EVENT_TYPE_COUNT, events_, FALSE, INFINITE);
    dwEvent -= WAIT_OBJECT_0;
    switch (dwEvent) {
      case EVENT_EXIT: {
        for (int i = 0; i < EVENT_TYPE_COUNT; i++)
          CloseHandle(events_[i]);
        closesocket(socket_);
        return;
      }
      case EVENT_RECEIVE: {
        WSANETWORKEVENTS network_events;
        if (WSAEnumNetworkEvents(socket_, events_[EVENT_RECEIVE], &network_events) == SOCKET_ERROR) {
          NLOG_E("WSAEnumNeworkEvents error : %d\n", WSAGetLastError());
          continue;
        }

        if (network_events.lNetworkEvents & FD_READ) {
          SOCKADDR_IN from;
          int fromlen = sizeof(from);
          std::unique_lock<std::recursive_mutex> lock(mutex_);
          int res = recvfrom(socket_, buf, sizeof(buf), 0, (struct sockaddr*) &from, &fromlen);
          if (res == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
              // ResetEvent(events_[EVENT_RECEIVE]);
              NLOG_E("WSAEWOULDBLCOK\n");
              continue;
            }
            std::cout << "socket error from recvfrom : " << err << std::endl;
            continue;
          } else if (res == 0) {
            continue;
          }
          StreamBuffer stbuf;
          stbuf.CopyFrom(buf, res);

          // for (int i = 0; i < stbuf.size(); i++) {
          //   printf("%02X ", (char)*(stbuf.buffer() + i));
          //   if (i && !(i % 16)) printf("\n");
          // } printf("\n");

          for (auto it = peers_.begin(); it != peers_.end(); it++) {
            if (it->second->GetSockAddrIn()->sin_addr.s_addr == from.sin_addr.s_addr)
              it->second->Push(stbuf);
          }
        }
        break;
      }
      case EVENT_SEND: {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        for (auto it = peers_.begin(); it != peers_.end(); it++) {
          while (true) {
            // pull packetized stream to participant
            StreamBuffer stbuf = it->second->Pull();
            if (stbuf.Empty())
              break;

            while (!stbuf.Empty()) {
              // for (int i = 0; i < stbuf.size(); i++) {
              //   printf("%02X ", (char)*(stbuf.buffer() + i));
              //   if (i && !(i % 16)) printf("\n");
              // } printf("\n");
              // std::cout << "sending " << stbuf.size() << "bytes" << std::endl;
              int res = sendto(socket_, stbuf.buffer(), stbuf.size(), 0,
                               (struct sockaddr*)it->second->GetSockAddrIn(), sizeof(SOCKADDR_IN));
              if (res == SOCKET_ERROR) {
                // TODO:
                // should handle WSAEWOULDBLOCK

                std::cout << "socket error from sendto: " << WSAGetLastError() << std::endl;
                break;
              }
              stbuf.Consume(res);
            }
          }
          ResetEvent(events_[EVENT_SEND]);
        }
        break;
      }
      case EVENT_EXPIRE: {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        for (auto it = peers_.begin(); it != peers_.end(); it++)
          it->second->Expire();
          
        break;
      }
      case EVENT_RESEND: {
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        for (auto it = peers_.begin(); it != peers_.end(); it++)
          it->second->Resend();
        break;
      }
      default:
        ;
    }
  }
}


} // namespace net


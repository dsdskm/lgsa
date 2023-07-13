#ifndef MARS_STREAM_CONTROLLER_IMPL_H
#define MARS_STREAM_CONTROLLER_IMPL_H

#include "NetCommon.h"
#include "StreamController.h"
#include "StreamBuffer.h"
#include "Participant.h"
#include <mutex>
#include <set>
#include <queue>
#include <map>
#include <string>

namespace net {

class StreamControllerImpl : public StreamController {

 public:
  enum EVENT_TYPE {
    EVENT_EXIT = 0,
    EVENT_RECEIVE,
    EVENT_SEND,
    EVENT_EXPIRE,
    EVENT_RESEND,
    EVENT_TYPE_COUNT
  };
  StreamControllerImpl(StreamSenderClient*);

  void Start() override;
  void Stop() override;
  void Shutdown() override;

  const int AddPeer(const std::string& ip_address,
		    StreamReceiverClient* client) override;
  void RemovePeer(const int peer_id) override;
  
  int Send(StreamType type, char* buf, size_t buflen, bool guaranteed) override;
  size_t Receive(const int peer_id, int stream_id, char* buf, size_t buflen) override;
  static DWORD Runner(void*);

 private:
  void Run();
  SOCKET socket_;
  HANDLE thread_;
  HANDLE timer_;
  HANDLE events_[EVENT_TYPE_COUNT];
  std::recursive_mutex mutex_;
  std::map<const int, Participant*> peers_;
  StreamSenderClient* sender_client_;
};
  
} // namespace net
#endif // MARS_STREAM_CONTROLLER_IMPL_H

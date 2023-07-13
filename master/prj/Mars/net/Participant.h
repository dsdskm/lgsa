#ifndef MARS_PARTICIPANT_H
#define MARS_PARTICIPANT_H

#include "NetCommon.h"
#include "StreamController.h"
#include "Packet.h"
#include <string>

namespace net {

class Participant {
 public:
  Participant(const std::string& ip_address,
              StreamReceiverClient* client);

  void Push(StreamBuffer& buffer);
  StreamBuffer Pull();
  void Expire();
  void Resend();
  
  size_t Send(StreamType type, char* buf, size_t buflen, bool guaranteed);
  size_t Receive(int stream_id, char* buf, size_t buflen);

  const std::string& ip_address() { return ip_address_; }
  const int peer_id() const { return peer_id_; }
  SOCKADDR_IN* GetSockAddrIn() { return &addr_; }

 private:
  const int peer_id_;
  std::string ip_address_;
  StreamReceiverClient *client_;
  SOCKADDR_IN addr_;
  PacketPacker packer_;
  PacketMerger merger_;
  Packetizer packetizer_;

  std::map<int, StreamBuffer> incoming_stream_;
};

} // namespace net

#endif  // MARS_PARTICIPANT_H

#include "Participant.h"
#include <iostream>
#include "../MarsLog.h"

namespace net {

static int g_peer_id = 0;

Participant::Participant(
    const std::string& ip_address, StreamReceiverClient* client)
    : peer_id_(++g_peer_id),
      ip_address_(ip_address),
      client_(client) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(STREAM_PORT);
  inet_pton(AF_INET, ip_address.c_str(), &addr_.sin_addr);
}

void Participant::Push(StreamBuffer& buffer) {
  static int chunk_id = 0;

  if (buffer.Empty())
    return;

  if (packer_.Feed(buffer)) {
    while (packer_.HasNextPacket()) {
      StreamBuffer packet = packer_.PopPacket();
      if (packet.Empty())
        break;

      PacketWrapper wrapper(packet);

      // ACK received
      switch (wrapper.packet_type()) {
      case PACKET_TYPE_ACK:
        packetizer_.ACKReceived(*((DWORD*)wrapper.body()));
        break;
      case PACKET_TYPE_AUDIO:
      case PACKET_TYPE_VIDEO: {
        packetizer_.SendACK(PacketWrapper(packet).packet_id());
        merger_.InsertPacket(packet);
        break;
      }
      default:
        NLOG_E("Unknown packet type received : %d\n", wrapper.packet_type());
      }
    }
  }

  // check if there's completed stream chunk
  while (DWORD merged = merger_.NextMerged()) {
    NLOG_D("merged %d\n", merged);
    StreamBuffer chunk = merger_.PopMerged(merged);
    if (chunk.Empty())
      continue;

    // for (int i = 0; i < chunk.size(); i++) {
    //   printf("%02X ", (char)*(chunk.buffer() + i));
    //   if (i && !(i % 16)) printf("\n");
    // } printf("\n");

    incoming_stream_.insert(std::make_pair(++chunk_id, chunk));
    if (client_) {
      NLOG_D("calling onreceive :  %d\n", chunk_id);
      client_->OnReceive(chunk.type(), peer_id(), chunk_id, chunk.size());
    }
  }
}

StreamBuffer Participant::Pull() {
  if (packetizer_.HasNextPacket())
    return packetizer_.PopPacket();
  return StreamBuffer();
}

void Participant::Expire() {
  int merger_score = merger_.Expire(); // give up old incomplete packet slots
  int packetizer_score = packetizer_.Expire(); // remove packets out of window

  int score = merger_score * packetizer_score;
  NLOG_I("Network score to %s : %d\n", ip_address_.c_str(), score);
  // FIXME: need to improve get network score
  client_->OnNetworkScore(score);
}

void Participant::Resend() {
  packetizer_.Resend();
}

size_t Participant::Send(StreamType type, char* buf, size_t buflen, bool guaranteed) {
  PacketType packet_type = PACKET_TYPE_UNKNOWN;
  switch (type) {
    case STREAM_AUDIO:
      packet_type = PACKET_TYPE_AUDIO;
      break;
    case STREAM_VIDEO:
      packet_type = PACKET_TYPE_VIDEO;
      break;
    default:
      ;
  }
  if (!buflen)
    return 0;

  StreamBuffer buffer;
  size_t copied = buffer.CopyFrom(buf, buflen);
  NLOG_D("sending %d\n", copied);
  if (!buffer.Empty())
    packetizer_.Feed(packet_type, buffer, guaranteed);
  return copied;
}

size_t Participant::Receive(int stream_id, char* buf, size_t buflen) {
  NLOG_D("receive!\n");
  auto it = incoming_stream_.find(stream_id);
  if (it == incoming_stream_.end()) {
    std::cout << "No stream received with such a stream_id : " << stream_id << std::endl;
    return 0;
  }
  if (buflen < it->second.size()) {
    std::cout << "Buffer size is too small to contain stream data" << std::endl;
    return 0;
  }
  size_t written = it->second.CopyTo(buf, buflen);
  incoming_stream_.erase(it);
  return written;
}


} // namespace net

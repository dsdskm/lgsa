#ifndef MARS_PACKET_H
#define MARS_PACKET_H

#include "NetCommon.h"
#include "StreamController.h"
#include "StreamBuffer.h"
#include <map>
#include <queue>
#include <list>
#include <memory>
                
namespace net {

// PACKET:
// size limit: 512
// +---------------------------------------------------------------------------+
// | PID(4) | MID(4) | MCNT(2) | MSEQ(2) | TYPE(2) | BLEN(2) | BODY            |
// +---------------------------------------------------------+                 |
// |                                                                           |
// |                                                                           |
// |                                                                  +--------+
// |                                                                  | CRC(2) |
// +------------------------------------------------------------------+--------+
// number in () means the count of bytes
// PID   : packet id
// MID   : media id (a group of packets)
// MCNT  : total count of packets in same media id
// MSEQ  : sequence id of the packet in same media id
// TYPE  : packet type (audio, video, ping, ack...)
// BLEN  : body length (without CRC)
// CRC   : crc checksum


#pragma once

struct PacketHeader {
  DWORD packet_id;
  DWORD media_id;
  WORD media_cnt;
  WORD media_seq;
  WORD packet_type;
  WORD body_len;
};

struct PacketFooter {
  WORD crc;
};

enum PacketType {
  PACKET_TYPE_UNKNOWN = 0,
  PACKET_TYPE_AUDIO,
  PACKET_TYPE_VIDEO,
  PACKET_TYPE_PING,
  PACKET_TYPE_ACK
};

class PacketWrapper {
 public:
  PacketWrapper(StreamBuffer& packet);

  DWORD packet_id() { return header_->packet_id; }
  DWORD media_id() { return header_->media_id; }
  WORD media_cnt() { return header_->media_cnt; }
  WORD media_seq() { return header_->media_seq; }
  WORD packet_type() { return header_->packet_type; }
  WORD body_len() { return header_->body_len; }
  char* body() { return body_; }
  WORD crc() { return crc_; }
 private:
  StreamBuffer& packet_;
  PacketHeader* header_;
  char* body_;
  WORD crc_;
};
  
///////////////////////////////////////////////////////////////////////////////////////
// PacketSlot
// accumulate packets in a group
class PacketSlot {
 public:
  PacketSlot(PacketType type, DWORD media_id, WORD media_count);

  PacketType type() const { return type_; }
  DWORD media_id() const { return media_id_; }
  WORD media_count() const { return media_count_; }
  size_t payload_size() const { return payload_size_; }

  bool Full() { return media_count_ <= packets_->size(); }
  bool Empty() { return packets_->empty(); }
  bool Obsolete();
  
  bool Insert(WORD media_seq, StreamBuffer& buffer);
  bool CopyPayloadTo(char* dest, size_t destlen);

 private:
  PacketType type_;
  DWORD media_id_;
  WORD media_count_;
  size_t payload_size_;
  int64_t timestamp_;
  std::shared_ptr<std::map<WORD, StreamBuffer>> packets_; // <MSEQ, StreamBuffer>
};

///////////////////////////////////////////////////////////////////////////////////////
// PacketMerger
// split packet group by media_id
class PacketMerger {
 public:
  PacketMerger();
  bool InsertPacket(StreamBuffer& packet);
  DWORD NextMerged();
  StreamBuffer PopMerged(DWORD media_id);

  int Expire();
 private:

  std::list<DWORD> merged_media_ids_;
  std::map<DWORD, PacketSlot> packet_slots_;  // <MID, PacketSlot>
};

///////////////////////////////////////////////////////////////////////////////////////
// PacketPacker
// split input vector into unit of packets
class PacketPacker {
 public:
  bool Feed(StreamBuffer& buf);
  bool HasNextPacket();
  StreamBuffer PopPacket();
  size_t PacketCount() { return packets_.size(); }
  
 private:
  StreamBuffer Split();

  std::vector<char> in_buffer_;
  std::queue<DWORD> broken_pid_;
  std::queue<StreamBuffer> packets_;
};

///////////////////////////////////////////////////////////////////////////////////////
// Packetizer
// split media chunk into packet units
class Packetizer {
 public:
  Packetizer()
      : packet_id_(0),
        media_id_(0) {}
  bool Feed(PacketType type, StreamBuffer& buf, bool guaranteed);
  bool HasNextPacket();
  StreamBuffer PopPacket();
  size_t PacketCount();

  void SendACK(DWORD packet_id);
  void ACKReceived(DWORD packet_id);

  int Expire();
  int Resend();
 private:
  StreamBuffer Packetize();

  
  DWORD packet_id_;
  DWORD media_id_;
  std::queue<StreamBuffer> packets_;
  struct SentPacket {
    int64_t sent_time;
    int64_t retry_after;
    StreamBuffer packet;
  };
  std::map<DWORD, SentPacket> sliding_window_;
};


} //namespace net
#endif

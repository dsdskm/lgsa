#include "Packet.h"
#include <algorithm>
#include <iostream>
#include "../MarsLog.h"

namespace net {

static const size_t PACKET_SIZE_LIMIT = 1024;
static const size_t PACKET_BODY_SIZE_LIMIT =
  (PACKET_SIZE_LIMIT - sizeof(PacketHeader) - sizeof(PacketFooter)) / 2 * 2;
static const int64_t PACKET_EXPIRY_INTERVAL = 100000000; // 10 seconds
static const int64_t PACKET_RETRY_INTERVAL = 1000000; // 100 millisecond

///////////////////////////////////////////////////////////////////////////////////////
// CRC16
class CRC16 {
 public:
  static unsigned short Check(unsigned char* buf, size_t len) {
    unsigned short crc = 0;
    for(int counter = 0; counter < len; counter++)
        crc = (crc << 8) ^ Get(((crc >> 8) ^ *buf++) & 0x00FF);
    return crc;
  }
 private:
  static unsigned short Get(unsigned short in) {
    return table_[in];
  }
  static unsigned short table_[256];
};

unsigned short CRC16::table_[256] = {
  0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
  0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
  0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
  0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
  0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
  0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
  0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
  0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
  0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
  0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
  0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
  0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
  0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
  0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
  0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
  0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
  0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
  0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
  0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
  0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
  0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
  0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
  0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
  0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
  0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
  0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
  0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
  0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
  0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
  0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
  0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
  0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};


static int64_t get_epoch_time() {
  FILETIME ftnow;
  GetSystemTimeAsFileTime(&ftnow);
  return (int64_t)ftnow.dwLowDateTime + (((int64_t)ftnow.dwHighDateTime) << 32LL);
}

PacketWrapper::PacketWrapper(StreamBuffer& packet)
    : packet_(packet) {
  header_ = (PacketHeader*)packet_.buffer();
  body_ = packet_.buffer() + sizeof(PacketHeader);
  crc_ = *((DWORD*)(body_ + header_->body_len));
}


///////////////////////////////////////////////////////////////////////////////////////
// PacketSlot
PacketSlot::PacketSlot(PacketType type, DWORD media_id, WORD media_count)
    : type_(type),
      media_id_(media_id),
      media_count_(media_count),
      payload_size_(0),
      packets_(new std::map<WORD, StreamBuffer>),
      timestamp_(get_epoch_time()) {
}

bool PacketSlot::Obsolete() {
  return ((get_epoch_time() - timestamp_) > PACKET_EXPIRY_INTERVAL);
}

// insert packet into the slot of media id
bool PacketSlot::Insert(WORD media_seq, StreamBuffer& buffer) {
  auto ret = packets_->insert(std::make_pair(media_seq, buffer));
  if (!ret.second) {
    std::cout << "Failed to insert a packet to slot" << std::endl;
    return false;
  }
  PacketHeader* header = (PacketHeader*)buffer.buffer();
  payload_size_ += header->body_len;
  return true;
}

bool PacketSlot::CopyPayloadTo(char* dest, size_t destlen) {

  if (destlen < payload_size()) {
    std::cout << "Buffer size is to small to copy payload to" << std::endl;
    return false;
  }
  if (!Full()) {
    std::cout << "Packetslot has not enough packets to complete a media unit" << std::endl;
    return false;
  }

  char *p = dest;
  for (WORD i = 0; i < media_count(); i++) {
    StreamBuffer stream_buf = (*packets_)[i];
    if (stream_buf.Empty()) {
      std::cout << "We have missing packets in the slot: " << i << std::endl;
      return false;
    }
    PacketHeader* header = (PacketHeader*)stream_buf.buffer();
    char* body = stream_buf.buffer() + sizeof(PacketHeader);
    memcpy(p, body, header->body_len);
    p += header->body_len;
  }

  if ((p - dest) != payload_size()) {
    std::cout << "mismatch between copied size(" << (p - dest) << ") and payload size("
              << payload_size() << ")" << std::endl;
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// PacketMerger
PacketMerger::PacketMerger() {
}

bool PacketMerger::InsertPacket(StreamBuffer& packet) {
  // packet is the complete packet
  // assert(packet.size() > sizeof(PacketHeader));
  // assert(packet.size() == sizeof(PacketHeader) + ((PacketHeader*)packet.buffer())->body_len + sizeof(PacketFooter));

  PacketHeader* header = (PacketHeader*)packet.buffer();
  PacketType type = (PacketType)header->packet_type;
  DWORD media_id = header->media_id;
  WORD media_count = header->media_cnt;

  auto r = packet_slots_.insert(
      std::make_pair(media_id, PacketSlot(type, media_id, media_count)));
  r.first->second.Insert(header->media_seq, packet);
  if (r.first->second.Full()) {
    merged_media_ids_.push_back(media_id);
    merged_media_ids_.sort();
    return true;
  }
  return !merged_media_ids_.empty();
}

DWORD PacketMerger::NextMerged() {
  if (merged_media_ids_.empty())
    return 0;
  return merged_media_ids_.front();
}

StreamBuffer PacketMerger::PopMerged(DWORD media_id) {
  // erase index
  auto it = std::find(merged_media_ids_.begin(), merged_media_ids_.end(), media_id);
  if (it == merged_media_ids_.end()) {
    std::cout << "Bad thing got happend in PopMerged" << std::endl;
    return StreamBuffer();
  }
  auto slot_it = packet_slots_.find(media_id);
  if (slot_it == packet_slots_.end()) {
    std::cout << "Bad thing got happend in PopMerged" << std::endl;
    return StreamBuffer();
  }
    
  size_t payload_size = slot_it->second.payload_size();

  StreamBuffer buffer;
  switch (slot_it->second.type()) {
    case PACKET_TYPE_AUDIO:
      buffer.set_type(STREAM_AUDIO);
      break;
    case PACKET_TYPE_VIDEO:
      buffer.set_type(STREAM_VIDEO);
      break;
    default:
      // FIXME:
      // we should handle other control packets either.
      return buffer;
  }

  buffer.Resize(payload_size);
  if (!slot_it->second.CopyPayloadTo(buffer.buffer(), buffer.size())) {
    // something got broken; do cleanup
    NLOG_E("packet slot seems to be broken; do clean up\n");
    merged_media_ids_.clear();
    packet_slots_.clear();
    return StreamBuffer();
  }

  merged_media_ids_.erase(it);

  //  Expire(media_id); // expire obsolete
  return buffer;
}

int PacketMerger::Expire() {
  std::map<DWORD, PacketSlot> replace;
  int total = packet_slots_.size();
  int count = 0;
  for (auto&  p : packet_slots_) {
    if (p.second.Obsolete()) {
      auto it = std::find(merged_media_ids_.begin(), merged_media_ids_.end(), p.first);
      if (it != merged_media_ids_.end())
        merged_media_ids_.erase(it);
      count++;
      continue;
    }
    replace.insert(p);
  }
  packet_slots_.swap(replace);
  return total ? 10 * (total - count) / total : 10;
}

///////////////////////////////////////////////////////////////////////////////////////
// PacketPacker
bool PacketPacker::Feed(StreamBuffer& buf) {
  size_t tail = in_buffer_.size();
  in_buffer_.resize(tail + buf.size());
  buf.CopyTo(in_buffer_.data() + tail, buf.size());
  while (true) {
    StreamBuffer packet = Split();
    if (packet.Empty())
      break;
    packets_.push(packet);
  }
  return !packets_.empty();
}

bool PacketPacker::HasNextPacket() {
  return !packets_.empty();
}

StreamBuffer PacketPacker::PopPacket() {
  StreamBuffer buf = packets_.front();
  packets_.pop();
  return buf;
}

// FIXME:
// reduce copy op here
StreamBuffer PacketPacker::Split() {
  // less than even header size
  if (in_buffer_.size() < sizeof(PacketHeader))
    return StreamBuffer();

  PacketHeader* header = (PacketHeader*)in_buffer_.data();
  size_t packet_size = sizeof(PacketHeader) + sizeof(PacketFooter) + header->body_len;
  if (packet_size > PACKET_SIZE_LIMIT) {
    // something wrong!! consume all received data and discard them
    NLOG_E("Packet corrupted. Wrong packet size: %d\n", packet_size);
    in_buffer_.resize(0);
    return StreamBuffer();
  }

  if (in_buffer_.size() < packet_size)
    return StreamBuffer();

  // extract packet
  StreamBuffer packet;
  packet.CopyFrom(in_buffer_.data(), packet_size);

  // crc check
  unsigned short* packet_crc = (unsigned short*)(packet.buffer() + sizeof(PacketHeader) + header->body_len);
  unsigned short crc = CRC16::Check((unsigned char*)packet.buffer(), sizeof(PacketHeader) + header->body_len);
  if (*packet_crc != crc) {
    NLOG_E("CRC check failed packet: %x, computed: %x\n", packet_crc, crc);
    in_buffer_.resize(0);
    return StreamBuffer();
  }

  // reset in_buffer
  if (in_buffer_.size() - packet_size > 0) {
    std::vector<char> tmp(in_buffer_.size() - packet_size);
    memcpy(tmp.data(), in_buffer_.data() + packet_size, packet_size);
    in_buffer_.swap(tmp);
  } else {
    in_buffer_.clear();
  }
  return packet;
}

///////////////////////////////////////////////////////////////////////////////////////
// Packetizer
bool Packetizer::Feed(PacketType type, StreamBuffer& buf, bool guaranteed) {
  DWORD media_id = ++media_id_;
  WORD media_cnt = (buf.size() + PACKET_BODY_SIZE_LIMIT) / PACKET_BODY_SIZE_LIMIT;

  NLOG_D("buf.size() : %d\n", buf.size());

  WORD media_seq = 0;
  while (!buf.Empty()) {
    PacketHeader header;
    header.packet_id = ++packet_id_;
    header.media_id = media_id;
    header.media_cnt = media_cnt;
    header.media_seq = media_seq++;
    header.packet_type = (WORD)type;
    header.body_len = buf.size() > PACKET_BODY_SIZE_LIMIT ? PACKET_BODY_SIZE_LIMIT : buf.size();

    // std::cout << "Packet header size=" << sizeof(header) << std::endl;
    StreamBuffer packet;
    packet.set_guaranteed(guaranteed);
    packet.Resize(header.body_len + sizeof(PacketHeader) + sizeof(PacketFooter));
    // std::cout << packet.size() << std::endl;

    memcpy(packet.buffer(), &header, sizeof(header));
    memcpy(packet.buffer() + sizeof(header), buf.buffer(), header.body_len);
    WORD* crc = (WORD*)(packet.buffer() + sizeof(header) + header.body_len);

    *crc = CRC16::Check((unsigned char*)packet.buffer(), sizeof(header) + header.body_len);

    buf.Consume(header.body_len);
    packets_.push(packet);
  }
  return !packets_.empty();
}

bool Packetizer::HasNextPacket() {
  return !packets_.empty();
}

StreamBuffer Packetizer::PopPacket() {
  StreamBuffer packet = packets_.front();
  PacketWrapper wrapper(packet);
  // put into sliding window to resend
  if (wrapper.packet_type() == PACKET_TYPE_AUDIO ||
      wrapper.packet_type() == PACKET_TYPE_VIDEO) {

    // real payload should be resent when it is not ack-ed
    SentPacket sent;
    sent.sent_time = get_epoch_time();
    sent.retry_after = sent.sent_time + PACKET_RETRY_INTERVAL;
    sent.packet = packet;
  
    sliding_window_.insert(std::make_pair(wrapper.packet_id(), sent));
  }
  packets_.pop();
  return packet;
}

void Packetizer::SendACK(DWORD packet_id) {
  // StreamBuffer ack(sizeof(PacketHeader) + sizeof(DWORD) + sizeof(PacketFooter));
  // PacketHeader* header = (PacketHeader*)ack.buffer();
  // header->packet_id = ++packet_id_;
  // header->media_id = 0;
  // header->media_cnt = 0;
  // header->media_seq = 0;
  // header->packet_type = (WORD)PACKET_TYPE_ACK;
  // header->body_len = sizeof(DWORD);

  // DWORD* body = (DWORD*)PacketWrapper(ack).body();
  // *body = packet_id;
  // DWORD* crc = (DWORD*)(ack.buffer() + sizeof(PacketHeader) + header->body_len);
  // *crc = CRC16::Check((unsigned char*)ack.buffer(), sizeof(PacketHeader) + header->body_len);
  // packets_.push(ack);
  PacketHeader header;
  header.packet_id = ++packet_id_;
  header.media_id = 0;
  header.media_cnt = 0;
  header.media_seq = 0;
  header.packet_type = (WORD)PACKET_TYPE_ACK;
  header.body_len = sizeof(packet_id);

  // std::cout << "Packet header size=" << sizeof(header) << std::endl;
  StreamBuffer packet;
  packet.Resize(header.body_len + sizeof(PacketHeader) + sizeof(PacketFooter));
  // std::cout << packet.size() << std::endl;

  memcpy(packet.buffer(), &header, sizeof(header));
  memcpy(packet.buffer() + sizeof(header), &packet_id, header.body_len);
  WORD* crc = (WORD*)(packet.buffer() + sizeof(header) + header.body_len);

  *crc = CRC16::Check((unsigned char*)packet.buffer(), sizeof(header) + header.body_len);
  packets_.push(packet);
}

void Packetizer::ACKReceived(DWORD packet_id) {
  auto it = sliding_window_.find(packet_id);
  if (it != sliding_window_.end())
    sliding_window_.erase(it);
}

int Packetizer::Expire() {
  int total = sliding_window_.size();
  int count = 0;
  std::map<DWORD, SentPacket> replace;
  int64_t now = get_epoch_time();
  for (auto& p : sliding_window_) {
    // All of packets sent before PACKET_EXPIRY_INTERVAL will be history
    if ((now - p.second.sent_time) < PACKET_EXPIRY_INTERVAL) {
      replace.insert(p);
      continue;
    }
    count++;
  }
  sliding_window_.swap(replace);
  return total ? 10 * (total - count) / total : 10;
  // NLOG_I("%lld : Expired %d packets\n", now, count);
}

int Packetizer::Resend() {
  int total = sliding_window_.size();
  int count = 0;
  for (auto& p : sliding_window_) {
    int64_t now = get_epoch_time();
    // Already retried no-guaranteed data.
    // We dont need to send this packet again. Just let it get expired
    if (p.second.retry_after == 0)
      continue;

    // try to resend
    if (p.second.retry_after < now) {
      packets_.push(p.second.packet); // resend
      // Applying different on QoS for a guaranteed chunk of data
      p.second.retry_after = p.second.packet.guaranteed() ? now + PACKET_RETRY_INTERVAL : 0;
      count++;
    } 
  }
  return total ? 10 * (total - count) / total : 10;
  // NLOG_I("Resending %d packets\n", count);
}

} //namespace net


#ifndef MARS_STREAM_BUFFER_H
#define MARS_STREAM_BUFFER_H

#include "NetCommon.h"
#include "StreamController.h"
#include <vector>
#include <memory>

namespace net {

class StreamBuffer {
 public:
  StreamBuffer();
  StreamBuffer(size_t s);
  virtual ~StreamBuffer() {}

  StreamType type() { return type_; }
  void set_type(StreamType type) { type_ = type; }
  char* buffer() { return buffer_->data() + offset_; }
  size_t size() { return buffer_->size() - offset_; }
  bool guaranteed() { return guaranteed_; }
  void set_guaranteed(bool g) { guaranteed_ = g; }

  void Resize(size_t len);
  size_t Consume(size_t len);
  size_t CopyFrom(char* src, size_t srclen);
  size_t CopyTo(char* dest, size_t destlen);
  bool Empty() { return size() <= 0; }

 private:
  StreamType type_;
  std::shared_ptr<std::vector<char>> buffer_;
  size_t offset_;
  bool guaranteed_;
};



} // namespace net

#endif // MARS_STREAM_BUFFER_H


#include "StreamBuffer.h"
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a > b) ? (b) : (a))
namespace net {

StreamBuffer::StreamBuffer()
    : type_(STREAM_DATA),
      buffer_(new std::vector<char>()),
      offset_(0),
      guaranteed_(false) {
}

StreamBuffer::StreamBuffer(size_t s)
    : type_(STREAM_DATA),
      buffer_(new std::vector<char>(s)),
      offset_(0),
      guaranteed_(false) {
}

void StreamBuffer::Resize(size_t len) {
  buffer_->resize(len);
  offset_ = 0;
}

size_t StreamBuffer::Consume(size_t len) {
  size_t m = MIN(len, size());
  offset_ += m;
  return m;
}

size_t StreamBuffer::CopyFrom(char* src, size_t srclen) {
  buffer_->resize(srclen);
  memcpy(buffer(), src, srclen);
  offset_ = 0;
  return size();
}

size_t StreamBuffer::CopyTo(char* dest, size_t destlen) {
  if (Empty())
    return 0;
  size_t len = MIN(destlen, size());
  memcpy(dest, buffer(), len);
  Consume(len);
  return len; 
}
  
} // namespace net

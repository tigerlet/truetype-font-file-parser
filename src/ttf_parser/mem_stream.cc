#include "mem_stream.h"
#include <cstring>

namespace ttf_dll {

void MemStream::ReadBigEndian(void *buf, unsigned size) {
  if (cursor_ + size > end_) {
    if (cursor_ < end_) {
      size_t avail = end_ - cursor_;
      char *c = static_cast<char*>(buf);
      for (int i = size - 1; i >= 0; --i) {
        if (static_cast<size_t>(size - 1 - i) < avail) {
          c[i] = *cursor_++;
        } else {
          c[i] = 0;
        }
      }
    } else {
      memset(buf, 0, size);
    }
    return;
  }
  char *c = static_cast<char*>(buf);
  for (int i = size - 1; i >= 0; --i) {
    c[i] = *cursor_++;
  }
}

void MemStream::ReadBigEndian(void *buf, unsigned unit_size, unsigned unit_num) {
  for (unsigned i = 0; i < unit_num; ++i) {
    ReadBigEndian(buf, unit_size);
    buf = static_cast<char*>(buf) + unit_size;
  }
}

void MemStream::Read(void *buf, size_t size) {
  if (cursor_ + size > end_) {
    if (cursor_ < end_) {
      size_t avail = end_ - cursor_;
      memcpy(buf, cursor_, avail);
      memset(static_cast<char*>(buf) + avail, 0, size - avail);
      cursor_ = end_;
    } else {
      memset(buf, 0, size);
    }
    return;
  }
  char *tmp = static_cast<char*>(buf);
  for (size_t i = 0; i < size; ++i) {
    *tmp++ = *cursor_++;
  }
}

void MemStream::Seek(int offset) {
  cursor_ += offset;
  if (cursor_ < start_) {
    cursor_ = start_;
  } else if (cursor_ > end_) {
    cursor_ = end_;
  }
}

int MemStream::Tell() {
  return cursor_ - start_;
}

}

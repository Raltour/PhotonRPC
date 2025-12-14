#include "TinyRPC/net/buffer.h"

#include <sys/socket.h>

Buffer::Buffer()
  : read_index_(0), write_index_(0) {
  buffer_ = std::make_unique<std::vector<char> >();
  // buffer_ = new std::vector<char>;
  buffer_->resize(1024);
}

void Buffer::WriteData(std::string& data, int size) {
  std::vector<char>::iterator iter = buffer_->begin() + write_index_;
  for (int i = 0; i < size; ++i) {
    // buffer_->push_back(data[i]);
    // buffer_->push_back(data.at(i));
    *iter = data[i];
    ++iter;
  }
  write_index_ += size;
}

void Buffer::RetrieveData(int size) {
  read_index_ += size;
}

bool Buffer::ReceiveFd(int fd) {
  int read_size = recv(fd, buffer_->data() + write_index_, buffer_->size(),
                       0);
  if (read_size > 0) {
    write_index_ += read_size;
    return true;
  } else {
    return false;
  }
}

bool Buffer::SendFd(int fd) {
  int send_size = send(fd, buffer_->data() + read_index_, GetSize(), 0);
  read_index_ += send_size;
  if (read_index_ == write_index_) {
    return true;
  } else {
    return false;
  }
}

const char* Buffer::PeekData() {
  return buffer_->data() + read_index_;
}

int Buffer::GetSize() {
  return write_index_ - read_index_;
}
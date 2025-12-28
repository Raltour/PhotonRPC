#include "photonrpc/net/buffer.h"

#include <sys/socket.h>

Buffer::Buffer() : read_index_(0), write_index_(0), data_size_(0) {
  buffer_ = std::make_unique<std::vector<char>>();
  // TODO: make better use of memory in buffer.
  buffer_->resize(1024);
}

void Buffer::WriteData(std::string& data, int size) {
  if (size > static_cast<int>(buffer_->size()) - write_index_ + read_index_) {
    buffer_->resize(buffer_->size() * 2);
  }
  std::vector<char>::iterator iter = buffer_->begin() + write_index_;
  for (int i = 0; i < size; ++i) {
    if (iter == buffer_->end()) {
      iter = buffer_->begin();
    }
    *iter = data[i];
    ++iter;
  }
  data_size_ += size;
  write_index_ = (write_index_ + size) % buffer_->size();
}

std::string Buffer::PeekData() const {
  int size = this->data_size_;
  std::string data;
  auto iter = buffer_->begin() + read_index_;
  for (int i = 0; i < size; ++i) {
    if (iter == buffer_->end()) {
      iter = buffer_->begin();
    }
    if (iter == buffer_->begin() + write_index_) {
      return "";
    }
    data.push_back(*iter);
    ++iter;
  }
  return data;
}

std::string Buffer::RetrieveData(int size) {
  std::string data;
  auto iter = buffer_->begin() + read_index_;
  for (int i = 0; i < size; ++i) {
    if (iter == buffer_->end()) {
      iter = buffer_->begin();
    }
    if (iter == buffer_->begin() + write_index_) {
      return "";
    }
    data.push_back(*iter);
    ++iter;
  }
  read_index_ = (read_index_ + size) % buffer_->size();
  data_size_ -= size;
  // return "";
  return data;
}

bool Buffer::ReceiveFd(int fd) {
  std::string temp;
  temp.resize(1024);
  int read_size = recv(fd, temp.data(), temp.size(), 0);
  if (read_size > 0) {
    this->WriteData(temp, read_size);
    return true;
  } else {
    return false;
  }
}

bool Buffer::SendFd(int fd) {
  std::string temp = this->RetrieveData(((write_index_ - read_index_) % buffer_->size() + buffer_->size()) % buffer_->size());
  // int send_size = send(fd, buffer_->data() + read_index_, GetSize(), 0);
  int send_size = send(fd, temp.data(), temp.size(), 0);
  // read_index_ += send_size;
  if (send_size > 0) {
    std::string remain_data = temp.substr(send_size);
    this->WriteData(remain_data, remain_data.size());
    return true;
  } else {
    return false;
  }

}

int Buffer::GetSize() const {
  return write_index_ - read_index_;
}

#include "photonrpc/net/buffer.h"

#include <sys/socket.h>

Buffer::Buffer() : read_index_(0), write_index_(0), data_size_(0) {
  buffer_ = std::make_unique<std::vector<char>>();
  buffer_->resize(1024);
}

Buffer::Buffer(int init_size) : read_index_(0), write_index_(0), data_size_(0) {
  buffer_ = std::make_unique<std::vector<char>>();
  buffer_->resize(init_size);
}

void Buffer::WriteData(std::string& data, int size) {
  if (size > static_cast<int>(buffer_->size()) - data_size_) {
    int original_size_index = buffer_->size();
    buffer_->resize(original_size_index * 2);
    if (write_index_ < read_index_) {
      for (auto iter = buffer_->begin(); iter != buffer_->begin() + write_index_; ++iter) {
        buffer_->at(original_size_index) = *iter;
        original_size_index++;
      }
      write_index_ = original_size_index;
    }
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
  std::string data;
  auto iter = buffer_->begin() + read_index_;
  for (int i = 0; i < data_size_; ++i) {
    if (iter == buffer_->end()) {
      iter = buffer_->begin();
    }
    data.push_back(*iter);
    ++iter;
  }
  return data;
}

bool Buffer::RetrieveData(int size) {
  if (data_size_ >= size) {
    read_index_ = (read_index_ + size) % buffer_->size();
    data_size_ -= size;
    return true;
  } else {
    return false;
  }
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
  std::string temp = this->PeekData();
  this->RetrieveData(data_size_);
  int send_size = send(fd, temp.data(), temp.size(), 0);
  if (send_size > 0) {
    std::string remain_data = temp.substr(send_size);
    this->WriteData(remain_data, remain_data.size());
    return true;
  } else {
    return false;
  }

}

int Buffer::GetSize() const {
  return data_size_;
}

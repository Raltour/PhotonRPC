#include "buffer.h"

#include <sys/socket.h>
#include <sys/uio.h>
#include <string>

Buffer::Buffer() : read_index_(0), write_index_(0), data_size_(0) {
  buffer_ = std::make_unique<std::vector<char>>();
  buffer_->resize(1024);
}

Buffer::Buffer(int init_size) : read_index_(0), write_index_(0), data_size_(0) {
  buffer_ = std::make_unique<std::vector<char>>();
  buffer_->resize(init_size);
}

void Buffer::WriteData(std::string& data, int size) {
  while (size >= static_cast<int>(buffer_->size()) - data_size_) {
    int original_size_index = buffer_->size();
    buffer_->resize(original_size_index * 2);
    if (write_index_ < read_index_) {
      for (auto iter = buffer_->begin();
           iter != buffer_->begin() + write_index_; ++iter) {
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
  int read_size = recv(fd, &temp[0], temp.size(), 0);
  if (read_size > 0) {
    this->WriteData(temp, read_size);
    return true;
  } else {
    return false;
  }
}

bool Buffer::SendFd(int fd) {
  if (data_size_ == 0) {
    return true;  // Nothing to send
  }

  // Get readable areas for writev
  auto area1 = GetReadableArea1();
  auto area2 = GetReadableArea2();
  
  struct iovec iov[2];
  int iov_count = 0;
  
  // Setup first area
  if (area1.first != nullptr && area1.second > 0) {
    iov[iov_count].iov_base = area1.first;
    iov[iov_count].iov_len = area1.second;
    iov_count++;
  }
  
  // Setup second area if exists
  if (area2.first != nullptr && area2.second > 0) {
    iov[iov_count].iov_base = area2.first;
    iov[iov_count].iov_len = area2.second;
    iov_count++;
  }
  
  if (iov_count == 0) {
    return true;  // Nothing to send
  }
  
  // Use writev for efficient sending
  ssize_t send_size = writev(fd, iov, iov_count);
  
  if (send_size > 0) {
    this->RetrieveData(send_size);
    return true;
  } else {
    return false;
  }
}

int Buffer::GetSize() const {
  return data_size_;
}

std::pair<char*, size_t> Buffer::GetReadableArea1() {
  if (data_size_ == 0) {
    return {nullptr, 0};
  }
  
  if (read_index_ < write_index_) {
    return {buffer_->data() + read_index_, data_size_};
  } else {
    return {buffer_->data() + read_index_, buffer_->size() - read_index_};
  }
}

std::pair<char*, size_t> Buffer::GetReadableArea2() {
  if (data_size_ == 0 || read_index_ < write_index_) {
    return {nullptr, 0};
  } else {
    return {buffer_->data(), write_index_};
  }
}

std::pair<char*, size_t> Buffer::GetWriteableArea1() {
  if (data_size_ == static_cast<int>(buffer_->size())) {
    return {nullptr, 0};  // Buffer is full
  }
  
  if (write_index_ < read_index_) {
    return {buffer_->data() + write_index_, read_index_ - write_index_};
  } else {
    return {buffer_->data() + write_index_, buffer_->size() - write_index_};
  }
}

std::pair<char*, size_t> Buffer::GetWriteableArea2() {
  if (data_size_ == static_cast<int>(buffer_->size()) || write_index_ < read_index_) {
    return {nullptr, 0};
  } else {
    return {buffer_->data(), read_index_};
  }
}

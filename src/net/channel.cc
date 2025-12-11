#include "TinyRPC/net/channel.h"

Channel::Channel(const int fd, bool read_event, bool write_event) {
  event_.data.fd = fd;
  if (read_event) {
    event_.events = EPOLLIN;
  }
  if (write_event) {
    event_.events |= EPOLLOUT;
  }
}

epoll_event* Channel::event() {
  return &event_;
}

void Channel::set_handle_read(std::function<void()> read_callback) {
  read_callback_ = read_callback;
}

void Channel::set_handle_write(std::function<void()> write_callback) {
  write_callback_ = write_callback;
}

void Channel::HandleRead() {
  this->read_callback_();
}

void Channel::HandleWrite() {
  this->write_callback_();
}
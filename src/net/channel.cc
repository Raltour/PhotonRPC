#include "channel.h"

#include <unistd.h>

// Channel::Channel(const int fd, bool read_event, bool write_event) {
//   event_ = {};
//   event_.data.fd = fd;
//   if (read_event) {
//     event_.events = EPOLLIN;
//   }
//   if (write_event) {
//     event_.events |= EPOLLOUT;
//   }
// }

Channel::Channel(const int fd) {
  event_ = {};
  event_.data.fd = fd;
}

epoll_event* Channel::event() {
  return &event_;
}
int Channel::fd() const {
  return event_.data.fd;
}

void Channel::set_handle_read(std::function<void()> read_callback) {
  read_callback_ = read_callback;
}

void Channel::set_handle_write(std::function<void()> write_callback) {
  write_callback_ = write_callback;
}

void Channel::enable_read_event() {
  event_.events |= EPOLLIN;
}

void Channel::enable_write_event() {
  event_.events |= EPOLLOUT;
}

void Channel::disable_read_event() {
  event_.events &= ~EPOLLIN;
}

void Channel::disable_write_event() {
  event_.events &= ~EPOLLOUT;
}

void Channel::HandleRead() {
  if (read_callback_) {
    this->read_callback_();
  }
}

void Channel::HandleWrite() {
  if (write_callback_) {
    this->write_callback_();
  }
}
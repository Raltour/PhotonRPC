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

// void Channel::HandleEvent() {
//   if (return_events_ & EPOLLIN) {
//     handle_read_();
//   } else if (return_events_ & EPOLLOUT) {
//     handle_write_();
//   }
// }

void Channel::set_handle_read(std::function<void()> read_callback) {
  handle_read_ = read_callback;
}

void Channel::set_handle_write(std::function<void()> write_callback) {
  handle_write_ = write_callback;
}

void Channel::read_callback() {
  this->handle_read_();
}

void Channel::write_callback() {
  this->handle_write_();
}
#include "TinyRPC/net/channel.h"

Channel::Channel(const int fd, bool read_event, bool write_event): fd_(fd), events_(0) {
  if (read_event) {
    events_|= EPOLLIN;
  }
  if (write_event) {
    events_|= EPOLLOUT;
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

void Channel::read_callback_() {
  this->handle_read_();
}

void Channel::write_callback() {
  this->handle_write_();
}
#include "poller.h"

#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <iostream>

#include "../common/logger.h"

// Poller::Poller() : return_events_(MAX_EVENT_NUMBER), epoll_fd_(epoll_create(MAX_EVENT_NUMBER)) {
Poller::Poller() : epoll_fd_(epoll_create(MAX_EVENT_NUMBER)) {
  if (this->epoll_fd_ == -1) {
    perror("epoll_create failed");
  }
}

Poller::~Poller() {
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
  }
}

int Poller::poll(int timeout) {
  int ret = epoll_wait(this->epoll_fd_, this->return_events_.data(),
                       MAX_EVENT_NUMBER, timeout);
  // int ret = epoll_wait(this->epoll_fd_, this->return_events_.data(), MAX_EVENT_NUMBER,
  //                      timeout);

  if (ret < 0) {
    if (errno == EINTR) {
      // LOG_DEBUG("poll error: EINTR, continue");
      return 0;
    }
    // LOG_ERROR("epoll failure");
    return -1;
  }
  return ret;
}

void Poller::RegisterChannel(Channel* channel) {
  int fd = channel->fd();

  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, channel->fd(),
                channel->event()) == -1) {
    perror("epoll_ctl add channel failed");
  }

  this->fd_channel_map_.emplace(fd, channel);
}

void Poller::RemoveChannel(Channel* channel) {
  int fd = channel->fd();
  this->fd_channel_map_.erase(fd);

  epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, channel->fd(), channel->event());
}

// std::vector<epoll_event>& Poller::get_return_events() {
//   return this->return_events_;
// }
epoll_event* Poller::get_return_events() {
  return this->return_events_.data();
}

Channel* Poller::get_channel_by_fd(int fd) {
  auto result = this->fd_channel_map_.find(fd);
  if (result != this->fd_channel_map_.end()) {
    return result->second;
  } else {
    return nullptr;
  }
}
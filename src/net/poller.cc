#include "TinyRPC/net/poller.h"
#include "TinyRPC/common/console_logger.h"

Poller::Poller() {
  this->epoll_fd_ = epoll_create(MAX_EVENT_NUMBER);
  this->fd_channel_map_.clear();
}

int Poller::poll(int timeout) {
  int ret = epoll_wait(this->epoll_fd_, this->return_events_, MAX_EVENT_NUMBER,
                       timeout);
  if (ret < 0) {
    LOG_ERROR("epoll failure");
    return -1;
  }
  return ret;
}

void Poller::register_channel(Channel* channel) {
  // this->fd_channel_map_.insert({channel.event_.data.fd, &channel});
  int fd = channel->event_.data.fd;
  this->fd_channel_map_.emplace(fd, channel);

  // epoll_event event;
  // event.data.fd = channel.fd_;
  // event.events = channel.events_;

  epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, channel->event_.data.fd,
            &channel->event_);
}

epoll_event* Poller::get_return_events() {
  return this->return_events_;
}

Channel* Poller::get_channel_by_fd(int fd) {
  return fd_channel_map_.find(fd)->second;
}
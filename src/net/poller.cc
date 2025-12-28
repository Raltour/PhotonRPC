#include "photonrpc/net/poller.h"
#include "photonrpc/common/console_logger.h"

Poller::Poller() {
  this->epoll_fd_ = epoll_create(MAX_EVENT_NUMBER);
  this->fd_channel_map_.clear();
}

int Poller::poll(int timeout) {
  int ret = epoll_wait(this->epoll_fd_, this->return_events_, MAX_EVENT_NUMBER,
                       timeout);
  if (ret < 0) {
    if (errno == EINTR) {
      LOG_DEBUG("poll error: EINTR, continue");
      return 0;
    }
    LOG_ERROR("epoll failure");
    return -1;
  }
  return ret;
}

void Poller::RegisterChannel(Channel* channel) {
  int fd = channel->event()->data.fd;
  this->fd_channel_map_.emplace(fd, channel);

  epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, channel->event()->data.fd,
            channel->event());
}

void Poller::RemoveChannel(Channel* channel) {
  int fd = channel->event()->data.fd;
  this->fd_channel_map_.erase(fd);

  epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, channel->event()->data.fd,
            channel->event());
}

epoll_event* Poller::get_return_events() {
  return this->return_events_;
}

Channel* Poller::get_channel_by_fd(int fd) {
  auto result = this->fd_channel_map_.find(fd);
  if (result != this->fd_channel_map_.end()) {
    return result->second;
  } else {
    return nullptr;
  }
}
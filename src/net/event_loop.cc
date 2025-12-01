#include "TinyRPC/net/event_loop.h"

EventLoop::EventLoop() {
  poller_ = Poller();
}

void EventLoop::Loop() {
  while (true) {
    int ret = poller_.poll(-1);
    if (ret < 0) {
      break;
    }

    epoll_event* result = poller_.get_return_events();

    for (int i = 0; i < ret; i++) {
      int sockfd = result[i].data.fd;
      int event_flag = result[i].events;
      Channel* channel = poller_.get_channel_by_fd(sockfd);

      if (event_flag & EPOLLIN) {
        channel->read_callback();
      }
      if (event_flag & EPOLLOUT) {
        channel->write_callback();
      }
    }
  }
}

void EventLoop::AddChannel(Channel* channel) {
  poller_.register_channel(channel);
}
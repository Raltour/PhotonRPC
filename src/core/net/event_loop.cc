#include "event_loop.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <csignal>
#include "../common/logger.h"

namespace {
EventLoop* event_loop = nullptr;
}

void stop_signal_handler(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    if (event_loop != nullptr) {
      event_loop->WakeUp();
    }
  }
}

EventLoop::EventLoop()
    : stopped_(false),
      wakeup_fd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeup_channel_({wakeup_fd_}) {

  // wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  // Channel* wakeup_channel = new Channel(wakeup_fd_, true, false);
  // wakeup_channel_(wakeup_fd_, true, false);

  wakeup_channel_.enable_read_event();
  wakeup_channel_.disable_read_event();
  wakeup_channel_.set_handle_read([this] {
    uint64_t one;
    int ret = read(wakeup_fd_, &one, sizeof(one));
    spdlog::info("Signal: Stoping Loop");
    stopped_ = true;
  });

  this->AddChannel(&wakeup_channel_);

  signal(SIGINT, stop_signal_handler);
  signal(SIGTERM, stop_signal_handler);
  event_loop = this;
}
EventLoop::~EventLoop() {
  close(wakeup_fd_);
}

void EventLoop::Loop() {
  spdlog::info("EventLoop start looping");
  while (!stopped_) {
    int ret = poller_.poll(-1);
    if (ret < 0) {
      break;
    }

    epoll_event* result = poller_.get_return_events();
    // std::vector<epoll_event>& result = poller_.get_return_events();

    if (ret > MAX_EVENT_NUMBER) {
      ret = MAX_EVENT_NUMBER;
      printf("Error: ret > MAX\n");
    }
    for (int i = 0; i < ret; i++) {
      // int sockfd = result.at(i).data.fd;
      // int event_flag = result.at(i).events;
      int sockfd = result[i].data.fd;
      int event_flag = result[i].events;
      Channel* channel = poller_.get_channel_by_fd(sockfd);

      if (channel == nullptr) {
        continue;
      }

      if (event_flag & EPOLLIN) {
        channel->HandleRead();
      }
      if (event_flag & EPOLLOUT) {
        channel->HandleWrite();
      }
    }
  }
  spdlog::info("EventLoop finish looping");
}

void EventLoop::AddChannel(Channel* channel) {
  poller_.RegisterChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  poller_.RemoveChannel(channel);
}

void EventLoop::WakeUp() {
  uint64_t one = 1;
  write(wakeup_fd_, &one, sizeof(one));
}
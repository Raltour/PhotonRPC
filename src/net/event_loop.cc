#include "photonrpc/net/event_loop.h"
#include "photonrpc/common/console_logger.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include <csignal>

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

EventLoop::EventLoop() : stopped_(false) {
  wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  Channel* wakeup_channel = new Channel(wakeup_fd_, true, false);
  wakeup_channel->set_handle_read([this] {
    LOG_DEBUG("wakeup channel read");
    uint64_t one;
    int ret = read(wakeup_fd_, &one, sizeof(one));
    LOG_DEBUG("Read from wakeup_fd_, size: " + std::to_string(ret));
    LOG_INFO("Signal: Stoping Loop");
    stopped_ = true;
    });

  this->AddChannel(wakeup_channel);

  signal(SIGINT, stop_signal_handler);
  signal(SIGTERM, stop_signal_handler);
  event_loop = this;
}

void EventLoop::Loop() {
  LOG_DEBUG("Starting Loop");
  while (!stopped_) {
    int ret = poller_.poll(-1);
    LOG_DEBUG("epoll return: " + std::to_string(ret));
    if (ret < 0) {
      break;
    }

    epoll_event* result = poller_.get_return_events();

    for (int i = 0; i < ret; i++) {
      int sockfd = result[i].data.fd;
      int event_flag = result[i].events;
      Channel* channel = poller_.get_channel_by_fd(sockfd);

      if (event_flag & EPOLLIN) {
        channel->HandleRead();
      }
      if (event_flag & EPOLLOUT) {
        channel->HandleWrite();
      }
    }
  }
  LOG_DEBUG("EventLoop::Loop Finish");
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
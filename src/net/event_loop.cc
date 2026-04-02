#include "event_loop.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <utility>

#include "../common/logger.h"

namespace {
EventLoop* event_loop = nullptr;
volatile std::sig_atomic_t stop_requested = 0;
}

void stop_signal_handler(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    if (event_loop != nullptr) {
      stop_requested = 1;
      event_loop->WakeUp();
    }
  }
}

EventLoop::EventLoop()
    : stopped_(false),
      wakeup_fd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeup_channel_({wakeup_fd_}),
      loop_thread_id_() {

  wakeup_channel_.enable_read_event();
  wakeup_channel_.disable_write_event();
  wakeup_channel_.set_handle_read([this] { HandleWakeUpRead(); });

  this->AddChannel(&wakeup_channel_);

  signal(SIGINT, stop_signal_handler);
  signal(SIGTERM, stop_signal_handler);
  event_loop = this;
}
EventLoop::~EventLoop() {
  if (event_loop == this) {
    event_loop = nullptr;
  }
  close(wakeup_fd_);
}

void EventLoop::Loop() {
  loop_thread_id_ = std::this_thread::get_id();
  LOG_INFO("EventLoop start looping");
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

    DoPendingTasks();
  }
  LOG_INFO("EventLoop finish looping");
}

void EventLoop::AddChannel(Channel* channel) {
  poller_.RegisterChannel(channel);
}

void EventLoop::UpdateChannel(Channel* channel) {
  poller_.UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  poller_.RemoveChannel(channel);
}

void EventLoop::WakeUp() {
  uint64_t one = 1;
  const ssize_t ret = write(wakeup_fd_, &one, sizeof(one));
  if (ret < 0 && errno != EAGAIN) {
    LOG_ERROR("EventLoop wakeup write failed, errno={}", errno);
  }
}

void EventLoop::QueueInLoop(Task task) {
  {
    std::lock_guard<std::mutex> lock(pending_tasks_mutex_);
    pending_tasks_.emplace_back(std::move(task));
  }
  WakeUp();
}

void EventLoop::RunInLoop(Task task) {
  if (IsInLoopThread()) {
    task();
    return;
  }
  QueueInLoop(std::move(task));
}

void EventLoop::Quit() {
  stopped_ = true;
  WakeUp();
}

bool EventLoop::IsInLoopThread() const {
  return loop_thread_id_ == std::this_thread::get_id();
}

void EventLoop::HandleWakeUpRead() {
  uint64_t one = 0;
  const ssize_t ret = read(wakeup_fd_, &one, sizeof(one));
  if (ret < 0) {
    return;
  }

  if (stop_requested != 0) {
    LOG_INFO("Signal: stopping loop");
    stopped_ = true;
    stop_requested = 0;
  }
}

void EventLoop::DoPendingTasks() {
  std::vector<Task> tasks;
  {
    std::lock_guard<std::mutex> lock(pending_tasks_mutex_);
    tasks.swap(pending_tasks_);
  }

  for (auto& task : tasks) {
    task();
  }
}

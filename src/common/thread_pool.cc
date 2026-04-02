#include "thread_pool.h"

#include <functional>
#include <utility>

ThreadPool::ThreadPool(size_t thread_count) {
  if (thread_count == 0) {
    thread_count = 1;
  }

  workers_.reserve(thread_count);
  for (size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back([this](std::stop_token stop_token) { WorkerLoop(stop_token); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    stopping_ = true;
  }

  for (auto& worker : workers_) {
    worker.request_stop();
  }
  task_semaphore_.release(static_cast<std::ptrdiff_t>(workers_.size()));
  workers_.clear();
}

void ThreadPool::WorkerLoop(std::stop_token stop_token) {
  while (true) {
    task_semaphore_.acquire();

    std::function<void()> task;
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (tasks_.empty()) {
        if (stop_token.stop_requested()) {
          break;
        }
        continue;
      }
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}

#include "thread_pool.h"

#include <mutex>

ThreadPool::ThreadPool() : done(false) {
  int thread_count = 4;

  for (int i = 0; i < thread_count; i++) {
    threads.emplace_back(&ThreadPool::worker_thread, this);
  }
}

ThreadPool::~ThreadPool() {
  done = true;
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }
}

template <typename FunctionType>
void ThreadPool::submit(FunctionType f) {
  std::unique_lock<std::mutex> lock(mtx);
  work_queue.push(std::function<void()>(f));
}

void ThreadPool::worker_thread() {
  while (!done) {
    std::unique_lock<std::mutex> lock(mtx);
    while (!work_queue.empty()) {
      auto task = work_queue.front();
      work_queue.pop();
      task();
    }
  }
}

#include "thread_pool.h"

ThreadPool::ThreadPool() : done(false) {
  int thread_count = 4;

  for (int i = 0; i < thread_count; i++) {
    threads.emplace_back(&ThreadPool::worker_thread, this);
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(mtx);
    done = true;
  }
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void ThreadPool::worker_thread() {
  while (!done) {
    while (!work_queue.empty()) {
      std::unique_lock<std::mutex> lock(mtx);
      if (!work_queue.empty()) {
        auto task = work_queue.front();
        work_queue.pop();
        task();
      }
    }
  }
}

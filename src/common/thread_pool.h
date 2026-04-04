#ifndef PHOTONRPC_THREAD_POOL_H
#define PHOTONRPC_THREAD_POOL_H

#include <atomic>
#include <cstddef>
#include <future>
#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(size_t thread_count);
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  template <typename F, typename... Args>
  auto Enqueue(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>;

  bool TryEnqueue(std::function<void()> task);

  size_t WorkerCount() const { return workers_.size(); }

 private:
  void WorkerLoop(std::stop_token stop_token);

  std::vector<std::jthread> workers_;
  std::queue<std::function<void()> > tasks_;
  mutable std::mutex queue_mutex_;
  std::counting_semaphore<1048576> task_semaphore_{0};
  bool stopping_ = false;
};

template <typename F, typename... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> {
  using ReturnType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

  auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  auto future = task->get_future();

  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (stopping_) {
      throw std::runtime_error("ThreadPool is stopping");
    }
    tasks_.emplace([task]() { (*task)(); });
  }

  task_semaphore_.release();
  return future;
}

#endif

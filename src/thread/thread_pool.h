#ifndef PHOTONRPC_THREAD_POOL_H
#define PHOTONRPC_THREAD_POOL_H

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  ThreadPool();

  ~ThreadPool();

  template <typename FunctionType>
  void submit(FunctionType f) {
    {
      std::unique_lock<std::mutex> lock(mtx);
      work_queue.push(std::function<void()>(f));
    }
  }

 private:
  std::atomic<bool> done;

  std::queue<std::function<void()>> work_queue;
  std::mutex mtx;

  std::vector<std::thread> threads;

  void worker_thread();
};

#endif  //PHOTONRPC_THREAD_POOL_H

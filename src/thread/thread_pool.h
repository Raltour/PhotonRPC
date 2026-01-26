#ifndef PHOTONRPC_THREAD_POOL_H
#define PHOTONRPC_THREAD_POOL_H

#include <atomic>
#include <functional>
#include <queue>
#include <thread>

class ThreadPool {
 public:
  ThreadPool();

  ~ThreadPool();

  template <typename FunctionType>
  void submit(FunctionType f);

 private:
  std::atomic<bool> done;

  std::queue<std::function<void()>> work_queue;
  std::mutex mtx;

  std::vector<std::thread> threads;

  void worker_thread();
};

#endif  //PHOTONRPC_THREAD_POOL_H

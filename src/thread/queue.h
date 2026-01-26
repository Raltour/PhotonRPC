#ifndef PHOTONRPC_QUEUE_H
#define PHOTONRPC_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Queue {
  private:
  mutable std::mutex mut;
  std::queue<T> data_queue;
  std::condition_variable data_cond;

  public:
  Queue() {}

  void Push(T data) {
    std::lock_guard<std::mutex> lock(mut);
    data_queue.push(std::move(data));
    data_cond.notify_one();
  }

  void WaitAndPop(T& value) {
    std::unique_lock<std::mutex> lock(mut);
    data_cond.wait(lock, [this] { return !data_queue.empty(); });
    value = std::move(data_queue.front());
    data_queue.pop();
  }

  std::shared_ptr<T> WaitAndPop() {
    std::unique_lock<std::mutex> lock(mut);
    data_cond.wait(lock, [this] { return !data_queue.empty(); });
    std::shared_ptr<T> res = std::make_shared<T>(std::move(data_queue.front()));
    data_queue.pop();
    return res;
  }

  bool TryPop(T& value) {
    std::lock_guard<std::mutex> lock(mut);
    if (data_queue.empty()) {
      return false;
    }
    value = std::move(data_queue.front());
    data_queue.pop();
    return true;
  }

  std::shared_ptr<T> TryPop() {
    std::lock_guard<std::mutex> lock(mut);
    if (data_queue.empty()) {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res = std::make_shared<T>(std::move(data_queue.front()));
    data_queue.pop();
    return res;
  }

  bool Empty() const {
    std::lock_guard<std::mutex> lock(mut);
    return data_queue.empty();
  }
};

#endif  //PHOTONRPC_QUEUE_H

#ifndef PHOTONRPC_EVENT_LOOP_H
#define PHOTONRPC_EVENT_LOOP_H

#include "poller.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class EventLoop {
 public:
  using Task = std::function<void()>;

  EventLoop();

  ~EventLoop();

  void Loop();

  void AddChannel(Channel* channel);

  void UpdateChannel(Channel* channel);

  void RemoveChannel(Channel* channel);

  void WakeUp();

  void QueueInLoop(Task task);

  void RunInLoop(Task task);

  void Quit();

 private:
  bool IsInLoopThread() const;

  void HandleWakeUpRead();

  void DoPendingTasks();

  Poller poller_;

  std::atomic<bool> stopped_;

  int wakeup_fd_;

  Channel wakeup_channel_;

  std::thread::id loop_thread_id_;
  std::vector<Task> pending_tasks_;
  std::mutex pending_tasks_mutex_;
};

#endif  //PHOTONRPC_EVENT_LOOP_H

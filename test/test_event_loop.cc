#include <gtest/gtest.h>

#include "../src/net/event_loop.h"

#include <future>
#include <thread>

TEST(EventLoopTest, RunInLoopExecutesQueuedTaskOnLoopThread) {
  EventLoop loop;
  std::promise<std::thread::id> task_thread_promise;
  auto task_thread_future = task_thread_promise.get_future();

  std::thread loop_thread([&loop] { loop.Loop(); });

  loop.RunInLoop([&loop, &task_thread_promise] {
    task_thread_promise.set_value(std::this_thread::get_id());
    loop.Quit();
  });

  EXPECT_EQ(task_thread_future.get(), loop_thread.get_id());

  loop_thread.join();
}

TEST(EventLoopTest, RunInLoopExecutesImmediatelyInsideLoopThread) {
  EventLoop loop;
  std::promise<bool> immediate_promise;
  auto immediate_future = immediate_promise.get_future();

  std::thread loop_thread([&loop] { loop.Loop(); });

  loop.QueueInLoop([&loop, &immediate_promise] {
    bool ran_immediately = false;
    loop.RunInLoop([&ran_immediately] { ran_immediately = true; });
    immediate_promise.set_value(ran_immediately);
    loop.Quit();
  });

  EXPECT_TRUE(immediate_future.get());

  loop_thread.join();
}

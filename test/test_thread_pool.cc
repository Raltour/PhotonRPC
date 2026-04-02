#include <gtest/gtest.h>

#include "../src/common/thread_pool.h"

#include <atomic>
#include <chrono>
#include <future>
#include <vector>

TEST(ThreadPoolTest, EnqueueReturnsFuture) {
  ThreadPool pool(4);
  std::vector<std::future<int>> futures;
  futures.reserve(100);

  for (int i = 0; i < 100; ++i) {
    futures.emplace_back(pool.Enqueue([i]() { return i * i; }));
  }

  long long sum = 0;
  for (auto& future : futures) {
    sum += future.get();
  }
  EXPECT_EQ(sum, 328350);
}

TEST(ThreadPoolTest, EnqueueSupportsVoidTask) {
  ThreadPool pool(3);
  std::atomic<int> count{0};
  std::vector<std::future<void>> futures;
  futures.reserve(64);

  for (int i = 0; i < 64; ++i) {
    futures.emplace_back(pool.Enqueue([&count]() {
      count.fetch_add(1, std::memory_order_relaxed);
    }));
  }

  for (auto& future : futures) {
    future.get();
  }
  EXPECT_EQ(count.load(std::memory_order_relaxed), 64);
}

TEST(ThreadPoolTest, DestructorDrainsQueuedTasks) {
  std::atomic<int> done{0};
  {
    ThreadPool pool(2);
    for (int i = 0; i < 40; ++i) {
      pool.Enqueue([&done]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        done.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }
  EXPECT_EQ(done.load(std::memory_order_relaxed), 40);
}

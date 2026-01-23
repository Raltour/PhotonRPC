#include "../include/photonrpc/rpc.h"
#include "protocol/calculate_service.pb.h"

#include <barrier>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

constexpr int NUM_OF_THREADS = 8;
constexpr int NUM_OF_CLIENT_PER_THREADS = 10;
constexpr int NUM_OF_REQUESTS_PER_CLIENT = 20000;


std::chrono::steady_clock::time_point start_time;
std::barrier start_barrier(NUM_OF_THREADS + 1, []() {
  start_time = std::chrono::steady_clock::now();
});

std::chrono::steady_clock::time_point end_time;
std::barrier end_barrier(NUM_OF_THREADS + 1, []() {
  end_time = std::chrono::steady_clock::now();
});


std::atomic<int> request_count(0);

std::mutex latency_mutex;
std::vector<uint32_t> latency;



void thread_func() {
  std::vector<std::unique_ptr<RpcChannel>> channels;
  std::vector<std::unique_ptr<rpc::CalculateService_Stub>> calculate_service_stubs;
  for (int i = 0; i < NUM_OF_CLIENT_PER_THREADS; ++i) {
    channels.emplace_back(std::make_unique<RpcChannel>());
    calculate_service_stubs.emplace_back(
        std::make_unique<rpc::CalculateService_Stub>(channels.back().get()));
  }

  int arg1 = 5;
  int arg2 = 6;
  rpc::AddRequest add_request;
  rpc::AddResponse add_response;
  add_request.set_a(arg1);
  add_request.set_b(arg2);

  int local_count = 0;

  std::vector<uint32_t> local_latency;

  start_barrier.arrive_and_wait();

  for (int i = 0; i < NUM_OF_REQUESTS_PER_CLIENT; i++) {
    for (auto& stub : calculate_service_stubs) {
      auto t0 = std::chrono::steady_clock::now();
      stub->Add(nullptr, &add_request, &add_response, nullptr);
      // int result = arg1 + arg2;
      auto t1 = std::chrono::steady_clock::now();
      local_latency.push_back(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()));
      ++local_count;
    }
  }

  end_barrier.arrive_and_wait();

  request_count += local_count;
  std::lock_guard<std::mutex> lock(latency_mutex);
  latency.insert(latency.end(), local_latency.begin(), local_latency.end());
}

static uint32_t percentile_us(std::vector<uint32_t>& v, double p) {
  // p: 0~100
  if (v.empty()) return 0;
  std::sort(v.begin(), v.end());
  double rank = (p / 100.0) * (v.size() - 1);
  size_t idx = (size_t)(rank + 0.5); // 四舍五入
  if (idx >= v.size()) idx = v.size() - 1;
  return v[idx];
}

int main() {

  std::thread threads[NUM_OF_THREADS];
  for (int i = 0; i < NUM_OF_THREADS; ++i) {
    threads[i] = std::thread(thread_func);
  }

  start_barrier.arrive_and_wait();

  end_barrier.arrive_and_wait();

  for (int i = 0; i < NUM_OF_THREADS; ++i) {
    threads[i].join();
  }

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  double qps = (request_count * 1000.0) / duration.count();

  std::cout << "Totle latency count: " << latency.size() << std::endl;
  uint32_t p50 = percentile_us(latency, 50);
  uint32_t p95 = percentile_us(latency, 95);
  uint32_t p99 = percentile_us(latency, 99);

  std::cout << "Total requests: " << request_count << std::endl;
  std::cout << "Total time: " << duration.count() << " ms" << std::endl;
  std::cout << "QPS: " << qps << std::endl;
  std::cout << "P50: " << p50 << " us" << std::endl;
  std::cout << "P95: " << p95 << " us" << std::endl;
  std::cout << "P99: " << p99 << " us" << std::endl;

  return 0;
}
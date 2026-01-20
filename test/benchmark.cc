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
constexpr int NUM_OF_CLIENT_PER_THREADS = 5;
constexpr int NUM_OF_REQUESTS_PER_CLIENT = 10000;


std::chrono::steady_clock::time_point start_time;
std::barrier start_barrier(NUM_OF_THREADS + 1, []() {
  start_time = std::chrono::steady_clock::now();
});

std::atomic<int> request_count(0);

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

  start_barrier.arrive_and_wait();

  for (int i = 0; i < NUM_OF_REQUESTS_PER_CLIENT; i++) {
    for (auto& stub : calculate_service_stubs) {
      stub->Add(nullptr, &add_request, &add_response, nullptr);
      ++request_count;
    }
  }

  request_count += local_count;
}

int main() {

  std::thread threads[NUM_OF_THREADS];
  for (int i = 0; i < NUM_OF_THREADS; ++i) {
    threads[i] = std::thread(thread_func);
  }

  start_barrier.arrive_and_wait();

  for (int i = 0; i < NUM_OF_THREADS; ++i) {
    threads[i].join();
  }

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  double qps = (request_count * 1000.0) / duration.count();

  std::cout << "Total requests: " << request_count << std::endl;
  std::cout << "Total time: " << duration.count() << " ms" << std::endl;
  std::cout << "QPS: " << qps << std::endl;

  return 0;
}
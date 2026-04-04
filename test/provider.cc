#include "../include/photonrpc/rpc.h"
#include "../src/common/logger.h"
#include "calculate_service.pb.h"
#include "echo_service.pb.h"

#include <chrono>
#include <csignal>
#include <memory>
#include <thread>

// Global pointer for signal handler
RpcServer* g_rpc_server = nullptr;

void SignalHandler(int signal) {
  LOG_INFO("Received signal {}, exiting...", signal);
  // Exit cleanly - the stack-allocated server will be cleaned up automatically
  exit(0);
}

class CalculateServiceImpl : public rpc::CalculateService {
 public:
  void Add(google::protobuf::RpcController* controller,
           const rpc::AddRequest* request, rpc::AddResponse* response,
           google::protobuf::Closure* done) override {
    response->set_result(request->a() + request->b());
  }

  void Sub(google::protobuf::RpcController* controller,
           const rpc::SubRequest* request, rpc::SubResponse* response,
           google::protobuf::Closure* done) override {
    response->set_result(request->a() - request->b());
  }
};

class EchoServiceImpl : public rpc::EchoService {
 public:
  void Echo(google::protobuf::RpcController* controller,
            const rpc::EchoRequest* request, rpc::EchoResponse* response,
            google::protobuf::Closure* done) override {
    if (request->sentence() == "__slow__") {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    response->set_result(request->sentence());
  }
};


int main() {
  Logger::Init();

  RpcServer rpc_server;

  rpc_server.ServiceRegister(std::make_unique<EchoServiceImpl>());
  rpc_server.ServiceRegister(std::make_unique<CalculateServiceImpl>());

  // Set up signal handler for cleanup
  g_rpc_server = &rpc_server;
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  rpc_server.StartServer();

  return 0;
}

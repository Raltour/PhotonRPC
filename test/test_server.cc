#include "TinyRPC/rpc/rpc_server.h"
#include "TinyRPC/protocol/rpc_message.pb.h"
#include "echo_service.pb.h"
#include "calculate_service.pb.h"
#include "TinyRPC/rpc/rpc_channel.h"
#include "TinyRPC/common/console_logger.h"

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
    response->set_result(request->sentence());
  }
};

int main() {
  RpcServer rpc_server;

  rpc_server.ServiceRegister(new EchoServiceImpl());
  rpc_server.ServiceRegister(new CalculateServiceImpl());

  rpc_server.StartServer();

  return 0;
}
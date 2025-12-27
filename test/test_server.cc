#include "TinyRPC/rpc/rpc_server.h"
#include "TinyRPC/protocol/rpc_message.pb.h"
#include "TinyRPC/protocol/add_service.pb.h"
#include "TinyRPC/rpc/rpc_channel.h"
#include "TinyRPC/common/console_logger.h"

class AddServiceImpl : public rpc::AddService {
 public:

  void Add(google::protobuf::RpcController* controller,
           const rpc::AddRequest* request, rpc::AddResponse* response,
           google::protobuf::Closure* done) override {
    response->set_result(request->a() + request->b());

    // done->Run();
  }

};

// std::string add(std::string arguments) {
//   rpc::AddRequest add_request;
//   add_request.ParseFromString(arguments);
//   rpc::AddResponse add_response;
//   add_response.set_result(add_request.a() + add_request.b());
//   LOG_DEBUG("Add result is: " + std::to_string(add_response.result()));
//   std::string response;
//   add_response.SerializeToString(&response);
//   return response;
// }

int main() {
  RpcServer rpc_server;

  AddServiceImpl add_service;

  rpc_server.ServiceRegister(new AddServiceImpl());

  rpc_server.StartServer();

  return 0;
}
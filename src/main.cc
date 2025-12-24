#include "TinyRPC/rpc/rpc_server.h"
#include "TinyRPC/protocol/rpc_message.pb.h"
#include "TinyRPC/protocol/add_service.pb.h"

std::string add(std::string arguments) {
  rpc::AddRequest add_request;
  add_request.ParseFromString(arguments);
  rpc::AddResponse add_response;
  add_response.set_result(add_request.a() + add_request.b());
  std::string response;
  add_response.SerializeToString(&response);
  return response;
}

int main() {
  RpcServer rpc_server;

  rpc_server.ServiceRegister("add", add);

  rpc_server.StartServer();

  return 0;
}
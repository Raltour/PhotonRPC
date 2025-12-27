#include <iostream>

#include "TinyRPC/protocol/add_service.pb.h"
#include "TinyRPC/rpc/rpc_channel.h"

int main() {
  RpcChannel channel;

  rpc::AddService_Stub add_service_stub(&channel);

  int arg1 = 5;
  int arg2 = 6;
  rpc::AddRequest add_request;
  rpc::AddResponse add_response;
  add_request.set_a(arg1);
  add_request.set_b(arg2);

  add_service_stub.Add(nullptr, &add_request, &add_response, nullptr);

  std::cout << "Received from server: " << add_response.result() << std::endl;

  return 0;
}
#include <iostream>

#include "calculate_service.pb.h"
#include "echo_service.pb.h"
#include "photonrpc/rpc/rpc_channel.h"

int main() {
  RpcChannel channel;

  rpc::EchoService_Stub echo_service_stub(&channel);
  rpc::CalculateService_Stub calculate_service_stub(&channel);

  rpc::EchoRequest echo_request;
  rpc::EchoResponse echo_response;
  echo_request.set_sentence("Hello, TinyRPC!");

  int arg1 = 5;
  int arg2 = 6;
  rpc::AddRequest add_request;
  rpc::AddResponse add_response;
  add_request.set_a(arg1);
  add_request.set_b(arg2);

  rpc::SubRequest sub_request;
  rpc::SubResponse sub_response;
  sub_request.set_a(arg1);
  sub_request.set_b(arg2);

  echo_service_stub.Echo(nullptr, &echo_request, &echo_response, nullptr);
  calculate_service_stub.Add(nullptr, &add_request, &add_response, nullptr);
  calculate_service_stub.Sub(nullptr, &sub_request, &sub_response, nullptr);

  std::cout << "Received from server: " << echo_response.result() << std::endl;
  std::cout << "Received from server: " << add_response.result() << std::endl;
  std::cout << "Received from server: " << sub_response.result() << std::endl;

  return 0;
}
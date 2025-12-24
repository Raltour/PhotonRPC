#include "TinyRPC/rpc/rpc_server.h"
#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/protocol/add_service.pb.h"
#include "TinyRPC/protocol/rpc_message.pb.h"

RpcServer::RpcServer()
    : tcp_server([this](std::string& read, std::string& write) {
        LOG_DEBUG("Reactor Service");
        this->HandleRequest(read, write);
      }) {}

void RpcServer::StartServer() {
  LOG_DEBUG("Start Server");
  tcp_server.RunLoop();
}

std::string add(std::string arguments) {
  rpc::AddRequest add_request;
  add_request.ParseFromString(arguments);
  rpc::AddResponse add_response;
  add_response.set_result(add_request.a() + add_request.b());
  std::string response;
  add_response.SerializeToString(&response);
  return response;
}

void RpcServer::HandleRequest(std::string& request, std::string& response) {
  rpc::RpcMessage request_message;
  request_message.ParseFromString(request);

  rpc::RpcMessage response_message;
  response_message.set_id(request_message.id());
  request_message.set_type(rpc::RPC_TYPE_RESPONSE);
  request_message.set_response(add(request_message.request()));
  request_message.SerializeToString(&response);
}

#include "TinyRPC/rpc/rpc_server.h"
#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/protocol/rpc_message.pb.h"

RpcServer::RpcServer()
    : tcp_server_([this](std::string& read, std::string& write) {
        LOG_DEBUG("Reactor Service");
        this->HandleRequest(read, write);
      }) {}

void RpcServer::StartServer() {
  LOG_DEBUG("Start Server");
  tcp_server_.RunLoop();
}

void RpcServer::ServiceRegister(std::string method_name,
                                std::function<std::string(std::string)> service_function) {
  service_map_.emplace(method_name, service_function);
}

void RpcServer::HandleRequest(std::string& request, std::string& response) {
  rpc::RpcMessage request_message;
  request_message.ParseFromString(request);

  rpc::RpcMessage response_message;
  response_message.set_id(request_message.id());
  request_message.set_type(rpc::RPC_TYPE_RESPONSE);
  request_message.set_response(service_map_[request_message.method_name()](request_message.request()));
  request_message.SerializeToString(&response);
}

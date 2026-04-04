#include "rpc_server.h"

#include "../common/logger.h"
#include "request_handler.h"

RpcServer::RpcServer() {
  // Initialize logger singleton
  // Logger::GetInstance();
  // Logger::Init();

  LOG_INFO("RpcServer start.");

  tcp_server_.SetUpTcpServer([this](const std::string& read,
                                    TcpConnection::SendResponse send_response) {
    send_response(RpcRequestHandler::HandleRequest(read, service_map_));
  });
}

void RpcServer::StartServer() {
  tcp_server_.RunLoop();
}

void RpcServer::ServiceRegister(
    std::unique_ptr<google::protobuf::Service> service) {
  service_map_.emplace(service->GetDescriptor()->name(), std::move(service));
}

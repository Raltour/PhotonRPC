#include "rpc_server.h"
#include "../common/logger.h"

RpcServer::RpcServer() {
  // Initialize logger singleton
  // Logger::GetInstance();
  // Logger::Init();

  LOG_INFO("RpcServer start.");

  tcp_server_.SetUpTcpServer([this](std::string& read, std::string& write) {
    this->HandleRequest(read, write);
  });
}

void RpcServer::StartServer() {
  tcp_server_.RunLoop();
}

void RpcServer::ServiceRegister(
    std::unique_ptr<google::protobuf::Service> service) {
  service_map_.emplace(service->GetDescriptor()->name(), std::move(service));
}

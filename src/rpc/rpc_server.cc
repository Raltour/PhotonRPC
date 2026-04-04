#include "rpc_server.h"

#include "../common/logger.h"
#include "request_handler.h"

namespace {
size_t GetDefaultWorkerCount() {
  const unsigned int worker_count = std::thread::hardware_concurrency();
  return worker_count == 0 ? 1U : worker_count;
}
}  // namespace

RpcServer::RpcServer() : thread_pool_(GetDefaultWorkerCount()) {
  // Initialize logger singleton
  // Logger::GetInstance();
  // Logger::Init();

  LOG_INFO("RpcServer start. worker_threads={}", thread_pool_.WorkerCount());

  tcp_server_.SetUpTcpServer([this](const std::string& read,
                                    TcpConnection::SendResponse send_response) {
    try {
      thread_pool_.Enqueue(
          [this, request = std::string(read),
           send_response = std::move(send_response)]() mutable {
            try {
              send_response(
                  RpcRequestHandler::HandleRequest(request, service_map_));
            } catch (const std::exception& ex) {
              LOG_ERROR("Rpc request handling failed: {}", ex.what());
              send_response(RpcRequestHandler::BuildErrorResponse(
                  request, "Internal server error"));
            } catch (...) {
              LOG_ERROR("Rpc request handling failed: unknown exception");
              send_response(RpcRequestHandler::BuildErrorResponse(
                  request, "Internal server error"));
            }
          });
    } catch (const std::exception& ex) {
      LOG_ERROR("Failed to enqueue rpc request: {}", ex.what());
      send_response(
          RpcRequestHandler::BuildErrorResponse(read, "Server is shutting down"));
    }
  });
}

void RpcServer::StartServer() {
  tcp_server_.RunLoop();
}

void RpcServer::ServiceRegister(
    std::unique_ptr<google::protobuf::Service> service) {
  service_map_.emplace(service->GetDescriptor()->name(), std::move(service));
}

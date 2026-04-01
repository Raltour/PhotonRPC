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

void RpcServer::HandleRequest(std::string& request, std::string& response) {
  rpc::RpcMessage request_message;
  request_message.ParseFromString(request);

  LOG_DEBUG("Received request: \n{}", request_message.DebugString());

  if (!CheckRequest(request_message)) {
    rpc::RpcMessage response_message;
    response_message.set_id(request_message.id());
    response_message.set_type(rpc::RPC_TYPE_ERROR);
    response_message.set_response("Invalid request");
    response_message.SerializeToString(&response);
    return;
  }

  auto service = service_map_.find(request_message.service_name())->second.get();
  auto service_desc = service->GetDescriptor();
  auto method_desc =
      service_desc->FindMethodByName(request_message.method_name());

  auto method_request = service->GetRequestPrototype(method_desc).New();
  auto method_response = service->GetResponsePrototype(method_desc).New();
  method_request->ParseFromString(request_message.request());

  service->CallMethod(method_desc, nullptr, method_request, method_response,
                      nullptr);

  rpc::RpcMessage response_message;
  response_message.set_id(request_message.id());
  response_message.set_type(rpc::RPC_TYPE_RESPONSE);
  response_message.set_response(method_response->SerializeAsString());
  response_message.SerializeToString(&response);

  delete method_request;
  delete method_response;

  LOG_DEBUG("Send response: \n{}", response_message.DebugString());
}

bool RpcServer::CheckRequest(rpc::RpcMessage request) {
  if (request.type() != rpc::RPC_TYPE_REQUEST) {
    LOG_ERROR("Invalid request type: {}", static_cast<int>(request.type()));
    return false;
  }

  if (request.method_name().empty()) {
    LOG_ERROR("Empty method name");
    return false;
  }

  if (request.service_name().empty()) {
    LOG_ERROR("Empty service name");
    return false;
  }

  if (service_map_.find(request.service_name()) == service_map_.end()) {
    LOG_ERROR("Service not found: {}", request.service_name());
    return false;
  }

  auto service = service_map_.find(request.service_name())->second.get();
  // if (service == nullptr) {
  //   LOG_ERROR("Service not found: {}", request.service_name());
  //   return false;
  // }

  auto service_desc = service->GetDescriptor();
  auto method_desc = service_desc->FindMethodByName(request.method_name());
  if (method_desc == nullptr) {
    LOG_ERROR("Method not found: {}", request.method_name());
    return false;
  }

  if (request.request().empty()) {
    LOG_ERROR("Empty request");
    return false;
  }

  return true;
}

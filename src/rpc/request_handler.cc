#include "request_handler.h"

#include "../common/logger.h"

namespace {
std::string BuildErrorResponseMessage(const rpc::RpcMessage* request_message,
                                      const std::string& error_message) {
  rpc::RpcMessage response_message;
  if (request_message != nullptr) {
    response_message.set_id(request_message->id());
  }
  response_message.set_type(rpc::RPC_TYPE_ERROR);
  response_message.set_response(error_message);

  std::string response;
  response_message.SerializeToString(&response);
  return response;
}
}  // namespace

std::string RpcRequestHandler::HandleRequest(const std::string& request,
                                             const ServiceMap& service_map) {
  std::string response;
  rpc::RpcMessage request_message;
  request_message.ParseFromString(request);

  LOG_DEBUG("Received request: \n{}", request_message.DebugString());

  if (!CheckRequest(request_message, service_map)) {
    return BuildErrorResponseMessage(&request_message, "Invalid request");
  }

  auto service = service_map.find(request_message.service_name())->second.get();
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
  return response;
}

std::string RpcRequestHandler::BuildErrorResponse(
    const std::string& request, const std::string& error_message) {
  rpc::RpcMessage request_message;
  if (!request_message.ParseFromString(request)) {
    return BuildErrorResponseMessage(nullptr, error_message);
  }
  return BuildErrorResponseMessage(&request_message, error_message);
}

bool RpcRequestHandler::CheckRequest(const rpc::RpcMessage& request,
                                     const ServiceMap& service_map) {
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

  if (service_map.find(request.service_name()) == service_map.end()) {
    LOG_ERROR("Service not found: {}", request.service_name());
    return false;
  }

  auto service = service_map.find(request.service_name())->second.get();
  auto service_desc = service->GetDescriptor();
  auto method_desc = service_desc->FindMethodByName(request.method_name());
  if (method_desc == nullptr) {
    LOG_ERROR("Method not found: {}", request.method_name());
    return false;
  }

  return true;
}

#ifndef PHOTONRPC_REQUEST_HANDLER_H
#define PHOTONRPC_REQUEST_HANDLER_H

#include <google/protobuf/service.h>

#include <map>
#include <memory>
#include <string>

#include "photonrpc/rpc_message.pb.h"

class RpcRequestHandler {
 public:
  using ServiceMap =
      std::map<std::string, std::unique_ptr<google::protobuf::Service>>;

  static std::string HandleRequest(const std::string& request,
                                   const ServiceMap& service_map);
  static std::string HandleRequestSafely(const std::string& request,
                                         const ServiceMap& service_map);
  static std::string BuildErrorResponse(const std::string& request,
                                        const std::string& error_message);

 private:
  static bool CheckRequest(const rpc::RpcMessage& request,
                           const ServiceMap& service_map);
};

#endif  // PHOTONRPC_REQUEST_HANDLER_H

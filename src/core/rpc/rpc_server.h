#ifndef PHOTONRPC_RPC_SERVER_H
#define PHOTONRPC_RPC_SERVER_H

#include <google/protobuf/service.h>

#include <string>
#include "../net/tcp_server.h"
#include "photonrpc/rpc_message.pb.h"

class RpcServer {
 public:
  RpcServer();

  ~RpcServer() = default;

  void StartServer();

  // void ServiceRegister(std::unique_ptr<google::protobuf::Service>);
  void ServiceRegister(google::protobuf::Service*);

 private:
  TcpServer tcp_server_;

  void HandleRequest(std::string& request, std::string& response);

  bool CheckRequest(rpc::RpcMessage request);

  std::map<std::string, google::protobuf::Service*> service_map_;
  // std::map<std::string, std::unique_ptr<google::protobuf::Service>> service_map_;
};

#endif  //PHOTONRPC_RPC_SERVER_H
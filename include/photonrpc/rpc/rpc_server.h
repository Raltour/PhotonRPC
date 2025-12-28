#ifndef TINYRPC_RPC_SERVER_H
#define TINYRPC_RPC_SERVER_H

#include <google/protobuf/service.h>

#include <string>
#include "photonrpc/net/tcp_server.h"
#include "photonrpc/protocol/rpc_message.pb.h"

// TODO: implement a tcp_client to maintain the tcp connection for better performance
class RpcServer {
 public:
  RpcServer();

  void StartServer();

  void ServiceRegister(google::protobuf::Service*);

 private:
  TcpServer tcp_server_;

  void HandleRequest(std::string& request, std::string& response);

  std::map<std::string, google::protobuf::Service*> service_map_;
};

#endif  //TINYRPC_RPC_SERVER_H
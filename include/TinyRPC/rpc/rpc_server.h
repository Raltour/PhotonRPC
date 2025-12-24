#ifndef TINYRPC_RPC_SERVER_H
#define TINYRPC_RPC_SERVER_H

#include "TinyRPC/protocol/rpc_message.pb.h"
#include "TinyRPC/net/tcp_server.h"
#include <string>

class RpcServer {
public:
  RpcServer();

  void StartServer();

  void ServiceRegister(std::string, std::function<std::string(std::string)>);

private:
  TcpServer tcp_server_;

  void HandleRequest(std::string& request, std::string& response);

  std::map<std::string, std::function<std::string(std::string)>> service_map_;

};


#endif //TINYRPC_RPC_SERVER_H
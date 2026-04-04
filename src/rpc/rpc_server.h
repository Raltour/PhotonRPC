#ifndef PHOTONRPC_RPC_SERVER_H
#define PHOTONRPC_RPC_SERVER_H

#include <google/protobuf/service.h>

#include <map>
#include <memory>
#include <string>

#include "../common/thread_pool.h"
#include "../net/tcp_server.h"

class RpcServer {
 public:
  RpcServer();

  ~RpcServer() = default;

  void StartServer();

  void ServiceRegister(std::unique_ptr<google::protobuf::Service> service);

 private:
  TcpServer tcp_server_;
  std::map<std::string, std::unique_ptr<google::protobuf::Service>> service_map_;
  ThreadPool thread_pool_;
};

#endif  //PHOTONRPC_RPC_SERVER_H

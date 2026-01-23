#ifndef PHOTONRPC_RPC_CHANNEL_H
#define PHOTONRPC_RPC_CHANNEL_H

#include "rpc_client.h"

#include <google/protobuf/service.h>

class RpcChannel : public google::protobuf::RpcChannel {
 public:
  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done) override;

 private:
  std::unique_ptr<RpcClient> rpc_client_ = std::make_unique<RpcClient>();
  // RpcClient* rpc_client_ = new RpcClient();
};

#endif  //PHOTONRPC_RPC_CHANNEL_H

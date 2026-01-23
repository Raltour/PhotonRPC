#include "rpc_channel.h"
#include "../common/config.h"
#include "../net/codec.h"
#include "photonrpc/rpc_message.pb.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {

  rpc::RpcMessage rpc_message;
  rpc_message.set_id(1);
  rpc_message.set_type(rpc::RPC_TYPE_REQUEST);
  rpc_message.set_service_name(method->service()->name());
  rpc_message.set_method_name(method->name());
  rpc_message.set_request(request->SerializeAsString());
  std::string message = rpc_message.SerializeAsString();

  std::string encoded_message = Codec::encode(message);
  rpc_client_->SendMessage(encoded_message);

  char buffer[1024];
  int read_size = rpc_client_->ReceiveMessage(buffer, 1024);
  if (read_size <= 0 || read_size >= 1024) {
    return;
  }
  buffer[read_size] = '\0';
  // TODO: Remove the magic number 4 here.
  std::string recv_data = std::string(buffer + 4);

  rpc_message.ParseFromString(recv_data);
  response->ParseFromString(rpc_message.response());
}
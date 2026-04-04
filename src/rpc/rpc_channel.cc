#include "rpc_channel.h"
#include "../common/config.h"
#include "../common/logger.h"
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
#include <vector>

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

  LOG_DEBUG("Calling method: {}.{}", rpc_message.service_name(), rpc_message.method_name());

  std::string encoded_message = Codec::encode(message);
  rpc_client_->SendMessage(encoded_message);

  int response_size = 0;
  if (!rpc_client_->ReceiveExact(reinterpret_cast<char*>(&response_size),
                                 sizeof(response_size))) {
    LOG_ERROR("Failed to receive response from server.");
    return;
  }

  if (response_size < 0) {
    LOG_ERROR("Received invalid response size: {}", response_size);
    return;
  }

  std::string recv_data(response_size, '\0');
  if (!rpc_client_->ReceiveExact(recv_data.data(), response_size)) {
    LOG_ERROR("Failed to receive complete response body from server.");
    return;
  }

  if (!rpc_message.ParseFromString(recv_data)) {
    LOG_ERROR("Failed to parse RpcMessage from received data.");
    return;
  }
  
  if (rpc_message.type() == rpc::RPC_TYPE_ERROR) {
    LOG_ERROR("RPC Error: {}", rpc_message.response());
    return;
  }

  if (!response->ParseFromString(rpc_message.response())) {
    LOG_ERROR("Failed to parse response message.");
    return;
  }
  
  LOG_DEBUG("RPC call success: {}.{}", method->service()->name(), method->name());
}

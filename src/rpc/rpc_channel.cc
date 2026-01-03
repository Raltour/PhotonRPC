#include "photonrpc/rpc/rpc_channel.h"
#include "photonrpc/net/codec.h"
#include "photonrpc/protocol/rpc_message.pb.h"
#include "photonrpc/common/config.h"

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
  const char* ip = Config::GetInstance().server_host().c_str();
  int port = Config::GetInstance().server_port();

  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &server_address.sin_addr);
  server_address.sin_port = htons(port);

  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
  assert(sockfd >= 0);

  if (connect(sockfd, (struct sockaddr*)&server_address,
              sizeof(server_address)) < 0) {
    printf("error!\n");
  }

  rpc::RpcMessage rpc_message;
  rpc_message.set_id(1);
  rpc_message.set_type(rpc::RPC_TYPE_REQUEST);
  rpc_message.set_service_name(method->service()->name());
  rpc_message.set_method_name(method->name());
  rpc_message.set_request(request->SerializeAsString());
  std::string message = rpc_message.SerializeAsString();

  std::string encoded_message = Codec::encode(message);
  send(sockfd, encoded_message.c_str(), encoded_message.size(), 0);

  char buffer[1024];
  int read_size = recv(sockfd, buffer, 1024, 0);
  buffer[read_size] = '\0';
  // TODO: Remove the magic number 4 here.
  std::string recv_data = std::string(buffer + 4);

  rpc_message.ParseFromString(recv_data);
  response->ParseFromString(rpc_message.response());

  close(sockfd);
}
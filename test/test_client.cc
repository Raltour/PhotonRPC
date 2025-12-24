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

#include "TinyRPC/net/codec.h"
#include "TinyRPC/protocol/add_service.pb.h"
#include "TinyRPC/protocol/rpc_message.pb.h"

int main() {
  const char* ip = "127.0.0.1";
  int port = 12345;

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

  int arg1 = 5;
  int arg2 = 6;
  rpc::RpcMessage request;
  request.set_id(1);
  request.set_type(rpc::RPC_TYPE_REQUEST);
  request.set_method_name("add");
  rpc::AddRequest add_request;
  add_request.set_a(arg1);
  add_request.set_b(arg2);
  request.set_request(add_request.SerializeAsString());
  std::string message = request.SerializeAsString();

  std::string encoded_message = Codec::encode(message);
  send(sockfd, encoded_message.c_str(), encoded_message.size(), 0);
  char buffer[1024];
  int read_size = recv(sockfd, buffer, 1024, 0);
  buffer[read_size] = '\0';
  std::string recv_data = std::string(buffer + 4);

  rpc::RpcMessage response;
  response.ParseFromString(recv_data);
  rpc::AddResponse add_response;
  add_response.ParseFromString(response.response());

  std::cout << "Received from server: " << add_response.result() << std::endl;

  close(sockfd);
  return 0;
}
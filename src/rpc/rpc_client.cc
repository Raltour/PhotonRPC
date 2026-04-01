#include "rpc_client.h"
#include "common/config.h"
#include "common/logger.h"

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

RpcClient::RpcClient() : sockfd_(-1) {
  ConnectToServer();
}

RpcClient::~RpcClient() {
  if (sockfd_ >= 0) {
    close(sockfd_);
    sockfd_ = -1;
  }
}

void RpcClient::ConnectToServer() {
  std::string host = Config::GetInstance().server_host();
  const char* ip = host.c_str();
  int port = Config::GetInstance().server_port();

  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &server_address.sin_addr);
  server_address.sin_port = htons(port);

  sockfd_ = socket(PF_INET, SOCK_STREAM, 0);
  assert(sockfd_ >= 0);

  if (connect(sockfd_, (struct sockaddr*)&server_address,
              sizeof(server_address)) < 0) {
    // printf("ConnectToServer error!\n");
    spdlog::error("Connec To Server error!");
    close(sockfd_);
    sockfd_ = -1;
  }
}

void RpcClient::SendMessage(const std::string& message) {
  if (send(sockfd_, message.c_str(), message.size(), 0) <= 0) {
    // printf("SendMessage error!\n");
    spdlog::error("Send Message error!");
  }
}

int RpcClient::ReceiveMessage(char* buffer, int size) {
  int read_size = recv(sockfd_, buffer, size, 0);
  return read_size;
}
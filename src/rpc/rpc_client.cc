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
  if (sockfd_ < 0) {
    LOG_ERROR("Create socket failed!");
    return;
  }

  if (connect(sockfd_, (struct sockaddr*)&server_address,
              sizeof(server_address)) < 0) {
    LOG_ERROR("Connect to server {}:{} failed!", ip, port);
    close(sockfd_);
    sockfd_ = -1;
  } else {
    LOG_INFO("Connected to server {}:{}", ip, port);
  }
}

void RpcClient::SendMessage(const std::string& message) {
  if (sockfd_ < 0) {
    LOG_ERROR("Cannot send message: not connected to server.");
    return;
  }
  if (send(sockfd_, message.c_str(), message.size(), 0) <= 0) {
    LOG_ERROR("Send message failed!");
  }
}

int RpcClient::ReceiveMessage(char* buffer, int size) {
  if (sockfd_ < 0) return -1;
  int read_size = recv(sockfd_, buffer, size, 0);
  if (read_size < 0) {
    LOG_ERROR("Receive message failed!");
  }
  return read_size;
}

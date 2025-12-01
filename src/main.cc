#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/net/tcp_server.h"
#include <iostream>

int main() {
  std::cout << "Hello, World!" << std::endl;
  LOG_INFO("Start Server");

  TcpServer server;

  return 0;
}
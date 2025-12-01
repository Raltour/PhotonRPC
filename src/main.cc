#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/net/tcp_server.h"
#include <iostream>

int main() {
  LOG_INFO("Start Server");

  TcpServer server;

  server.RunLoop();

  return 0;
}
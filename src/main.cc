#include <cstring>

#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/net/tcp_server.h"

int main() {
  LOG_INFO("Start Server");

  TcpServer server(
    [] (char* read, char* write) {
      LOG_DEBUG("Reactor Service");
      memcpy(write, read, strlen(read));
    }
  );

  server.RunLoop();

  return 0;
}
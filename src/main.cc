#include <cstring>

#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/net/tcp_server.h"

int main() {
  LOG_INFO("Start Server");

  TcpServer server(
      [](std::string& read, std::string& write) {
        LOG_DEBUG("Reactor Service");
        write = read;
        // memcpy(write, read, strlen(read));
        // write = "12345";
      }
      );

  server.RunLoop();

  return 0;
}
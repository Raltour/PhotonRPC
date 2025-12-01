#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/common/config.h"
#include "TinyRPC/net/channel.h"
#include <iostream>

int main() {
  std::cout << "Hello, World!" << std::endl;
  LOG_INFO("Start Server");

  Config::GetInstance();

  Channel test_channel(3, true, true);

  return 0;
}
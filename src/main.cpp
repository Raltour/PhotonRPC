#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/common/config.h"
#include <iostream>

int main() {
  std::cout << "Hello, World!" << std::endl;
  LOG_INFO("Start Server");

  Config::GetInstance();

  return 0;
}
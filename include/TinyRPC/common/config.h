#ifndef TINYRPC_CONFIG_H
#define TINYRPC_CONFIG_H

#include <string>

class Config {
public:
  static Config& GetInstance();

  std::string server_host_;
  int server_port_;

private:
  Config();
};


#endif //TINYRPC_CONFIG_H
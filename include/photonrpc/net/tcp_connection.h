#ifndef PHOTONRPC_TCP_CONNECTION_H
#define PHOTONRPC_TCP_CONNECTION_H

#include "photonrpc/net/buffer.h"
#include "photonrpc/net/channel.h"

#include <string>

class TcpConnection {
 public:
  TcpConnection(int connect_fd,
                std::function<void(std::string&, std::string&)> service,
                std::function<void(Channel*)> add_connection_callback);

  TcpConnection() = delete;

  void set_close_callback(std::function<void(Channel*)> close_callback);

 private:
  Channel channel_;

  const int max_buffer_size = 1024;
  Buffer input_buffer_;
  Buffer output_buffer_;

  // 注册给epoll的函数
  void HandleRead();

  //注册给epoll的函数
  void HandleWrite();

  // std::function<void(char* read, char* write)> service_;
  std::function<void(std::string& read, std::string& write)> service_;
  std::function<void(Channel*)> add_connection_callback_;
  std::function<void(Channel*)> close_callback_;
};

#endif  //PHOTONRPC_TCP_CONNECTION_H
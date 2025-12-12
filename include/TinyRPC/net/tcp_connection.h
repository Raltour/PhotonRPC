#ifndef TINYRPC_TCP_CONNECTION_H
#define TINYRPC_TCP_CONNECTION_H

#include <string>

#include "TinyRPC/net/channel.h"

class TcpConnection {
public:
  TcpConnection(int connect_fd, std::function<void(char*, char*)> service,
                std::function<void(Channel*)> add_connection_callback);

  TcpConnection() = delete;

  ~TcpConnection();

  void set_close_callback(std::function<void(Channel*)> close_callback);

private:
  Channel channel_;

  const int max_buffer_size = 1024;
  char* read_buffer_;
  char* write_buffer_;

  void HandleRead();

  void HandleWrite();

  std::function<void(char* read, char* write)> service_;
  std::function<void(Channel*)> add_connection_callback_;
  std::function<void(Channel*)> close_callback_;

};


#endif //TINYRPC_TCP_CONNECTION_H
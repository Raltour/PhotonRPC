#ifndef TINYRPC_TCP_CONNECTION_H
#define TINYRPC_TCP_CONNECTION_H

#include "TinyRPC/net/channel.h"

class TcpConnection {
public:
  TcpConnection(int connect_fd, std::function<void()> read_callback,
                std::function<void()> write_callback,
                std::function<void(Channel*)> add_connection_callback);

  TcpConnection() = delete;

  // void set_add_connection_callback(std::function<void(Channel*)> callback);

  // void set_read_task_callback(std::function<void()> callback);

  // void set_write_task_callback(std::function<void()> callback);


private:
  Channel channel_;

  void set_work(std::function<void()> read_callback,
                std::function<void()> write_callback,
                std::function<void(Channel*)> add_connection_callback);

  std::function<void(Channel*)> add_connection_callback_;

  // std::function<void()> read_task_callback_;

  // std::function<void()> write_task_callback_;
};


#endif //TINYRPC_TCP_CONNECTION_H
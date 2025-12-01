#ifndef TINYRPC_TCP_SERVER_H
#define TINYRPC_TCP_SERVER_H

#include "TinyRPC/net/event_loop.h"
#include "TinyRpc/net/acceptor.h"
#include "TinyRPC/net/tcp_connection.h"

#include <memory>

class TcpServer {
 public:
  TcpServer();
  void RunLoop();
  void AddChannel(Channel* channel);

 private:
  EventLoop event_loop_;
  Acceptor acceptor_;
  std::map<int, std::unique_ptr<TcpConnection>>  fd_connection_map_;

  std::function<void()> reactor_read_;
  std::function<void()> reactor_write_;
};

#endif  //TINYRPC_TCP_SERVER_H
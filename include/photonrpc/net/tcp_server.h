#ifndef TINYRPC_TCP_SERVER_H
#define TINYRPC_TCP_SERVER_H

#include "photonrpc/net/acceptor.h"
#include "photonrpc/net/event_loop.h"
#include "photonrpc/net/tcp_connection.h"

#include <memory>

class TcpServer {
 public:
  TcpServer(std::function<void(std::string&, std::string&)> service);

  TcpServer() = delete;

  void RunLoop();

  void AddChannel(Channel* channel);

 private:
  EventLoop event_loop_;

  Acceptor acceptor_;
  std::map<int, std::unique_ptr<TcpConnection>> fd_connection_map_;
};

#endif  //TINYRPC_TCP_SERVER_H
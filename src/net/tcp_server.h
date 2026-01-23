#ifndef PHOTONRPC_TCP_SERVER_H
#define PHOTONRPC_TCP_SERVER_H

#include "acceptor.h"
#include "event_loop.h"
#include "tcp_connection.h"

#include <memory>

class TcpServer {
 public:
  TcpServer() = default;

  void SetUpTcpServer(std::function<void(std::string&, std::string&)> service);

  void RunLoop();

 private:
  EventLoop event_loop_;

  Acceptor acceptor_;
  std::map<int, std::unique_ptr<TcpConnection>> fd_connection_map_;
};

#endif  //PHOTONRPC_TCP_SERVER_H
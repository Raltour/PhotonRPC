#include "TinyRPC/net/tcp_server.h"

#include "TinyRPC/common/console_logger.h"

TcpServer::TcpServer() {
  acceptor_.set_start_listen_callback([this](Channel* channel) {
    LOG_DEBUG("Acceptor called listen_callback");
    event_loop_.AddChannel(channel);
  });

  acceptor_.set_new_connection_callback([this](int connect_fd) {
    fd_connection_map_.insert({
      connect_fd,
      std::make_unique<TcpConnection>(connect_fd)});
  });
}

void TcpServer::RunLoop() {
  event_loop_.Loop();
}

void TcpServer::AddChannel(Channel* channel) {
  event_loop_.AddChannel(channel);
}

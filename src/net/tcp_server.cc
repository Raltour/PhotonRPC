#include "tcp_server.h"

#include <utility>

#include "../common/logger.h"

void TcpServer::SetUpTcpServer(TcpConnection::MessageCallback service) {
  acceptor_.set_start_listen_callback([this](Channel* channel) {
    LOG_DEBUG("Acceptor called listen_callback");
    event_loop_.AddChannel(channel);
  });

  acceptor_.set_new_connection_callback([this, service = std::move(service)](
                                            int connect_fd) mutable {
    auto tcp_connection = std::make_shared<TcpConnection>(
        connect_fd, &event_loop_, service,
        [this](int fd) { fd_connection_map_.erase(fd); });

    fd_connection_map_[connect_fd] = tcp_connection;
    tcp_connection->ConnectEstablished();
    LOG_INFO("TcpServer created new TcpConnection for fd: {}", connect_fd);
  });

  acceptor_.StartListen();
}

void TcpServer::RunLoop() {
  event_loop_.Loop();
}

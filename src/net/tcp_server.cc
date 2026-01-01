#include "photonrpc/net/tcp_server.h"

#include "photonrpc/common/logger.h"

TcpServer::TcpServer(std::function<void(std::string&, std::string&)> service) {

  acceptor_.set_start_listen_callback([this](Channel* channel) {
    LOG_DEBUG("Acceptor called listen_callback");
    event_loop_.AddChannel(channel);
  });

  acceptor_.set_new_connection_callback([this, service](int connect_fd) {
    fd_connection_map_.insert(
        {connect_fd, std::make_unique<TcpConnection>(
                         connect_fd, service, [this](Channel* channel) {
                           event_loop_.AddChannel(channel);
                         })});
    fd_connection_map_.find(connect_fd)
        ->second->set_close_callback([this](Channel* channel) {
          this->event_loop_.RemoveChannel(channel);
          this->fd_connection_map_.erase(channel->event()->data.fd);
        });
    LOG_INFO("TcpServer created new TcpConnection for fd: {}", connect_fd);
  });

  acceptor_.StartListen();
}

void TcpServer::RunLoop() {
  event_loop_.Loop();
}

void TcpServer::AddChannel(Channel* channel) {
  event_loop_.AddChannel(channel);
}
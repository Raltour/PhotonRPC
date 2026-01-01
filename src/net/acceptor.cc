#define MAX_LISTEN 64

#include "photonrpc/net/acceptor.h"
#include "photonrpc/common/config.h"
#include "photonrpc/common/logger.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

Acceptor::Acceptor() {}

void Acceptor::StartListen() {
  const char* ip = Config::GetInstance().server_host_.c_str();
  int port = Config::GetInstance().server_port_;

  struct sockaddr_in address;
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  this->listenfd_ = socket(PF_INET, SOCK_STREAM, 0);
  if (listenfd_ < 0) {
    LOG_ERROR("create listen_fd failure");
    return;
  }

  // Enable the bind function to reuse the same port.
  // In some case, the socket haven't been released by the Linux OS, but the server run again.
  int opt = 1;
  if (setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    LOG_ERROR("setsockopt failure");
  }

  int ret = bind(listenfd_, (struct sockaddr*)&address, sizeof(address));
  if (ret < 0) {
    LOG_ERROR("bind listen_fd failure, errno = " +
              std::string(strerror(errno)));
    exit(1);
  }

  ret = listen(listenfd_, MAX_LISTEN);
  if (ret < 0) {
    LOG_ERROR("listen failure");
    return;
  }

  LOG_INFO("Acceptor start listening on {}:{}", ip, port);

  listen_channel = Channel(listenfd_, true, false);
  listen_channel.set_handle_read([this] {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd =
        accept(listenfd_, (struct sockaddr*)&client_addr, &client_addr_len);
    if (connfd < 0) {
      LOG_ERROR("accept failure");
      return;
    }

    LOG_INFO("Acceptor accepted new connection from {}:{}, fd: {}",
             inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
             connfd);
    this->new_connection_callback_(connfd);
  });

  this->start_listen_callback_(&listen_channel);
}

void Acceptor::set_new_connection_callback(std::function<void(int)> callback) {
  new_connection_callback_ = callback;
}

void Acceptor::set_start_listen_callback(
    std::function<void(Channel*)> callback) {
  start_listen_callback_ = callback;
}
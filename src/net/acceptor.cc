#define MAX_LISTEN = 64

#include "TinyRPC/net/acceptor.h"
#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/common/config.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

Acceptor::Acceptor() {
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

  int ret = bind(listenfd_, (struct sockaddr *) &address, sizeof(address));
  if (ret < 0) {
    LOG_ERROR("bind listen_fd failure");
    return;
  }

  listen_channel = Channel(listenfd_, true, false);
  listen_channel.set_handle_read([this] {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd = accept(listenfd_, (struct sockaddr *) &client_addr, &client_addr_len);
    if (connfd < 0) {
      LOG_ERROR("accept failure");
      return;
    }

    this->new_connection_callback_(connfd);
  });


}

void Acceptor::set_new_connection_callback(std::function<void(int)> callback) {
  new_connection_callback_ = callback;
}

void Acceptor::set_start_listen_callback(std::function<void(Channel*)> callback) {
  start_listen_callback_ = callback;
}

// void Acceptor::listen_loop() {
//   while (true) {
//     struct sockaddr_in client_addr;
//     socklen_t client_addr_len = sizeof(client_addr);
//     int connfd = accept(listenfd_, (struct sockaddr *) &client_addr, &client_addr_len);
//     if (connfd < 0) {
//       LOG_ERROR("accept failure");
//       return;
//     }
//     new_connection_callback_();
//   }
// }
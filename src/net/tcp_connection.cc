#include "TinyRPC/net/tcp_connection.h"

#include <unistd.h>
#include <sys/socket.h>

#include "TinyRPC/common/console_logger.h"


TcpConnection::TcpConnection(int connect_fd, std::function<void(char*, char*)> service,
                             std::function<void(Channel*)>
                             add_connection_callback)
  : service_(service) {
  read_buffer_ = new char[max_buffer_size];
  write_buffer_ = new char[max_buffer_size];

  channel_ = Channel(connect_fd, true, false);
  channel_.set_handle_read([this] {
    LOG_DEBUG("TcpConnection HandleRead called");
    this->HandleRead();
  });
  channel_.set_handle_write([this] {
    LOG_DEBUG("TcpConnection HandleWrite called");
    this->HandleWrite();
  });
  this->add_connection_callback_ = add_connection_callback;
  LOG_DEBUG("TcpConnection called add_connection_callback");
  add_connection_callback_(&channel_);

}

TcpConnection::~TcpConnection() {
  free(write_buffer_);
  free(read_buffer_);
}

void TcpConnection::set_close_callback(
    std::function<void(Channel*)> close_callback) {
  close_callback_ = close_callback;
}


void TcpConnection::HandleRead() {
  int read_size = recv(channel_.event()->data.fd, read_buffer_, max_buffer_size,
                       0);
  if (read_size > 0) {
    LOG_DEBUG("TcpConnection received data");
    service_(read_buffer_, write_buffer_);
    HandleWrite();
  } else {
    LOG_DEBUG("TcpConnection connection closed");
    close(channel_.event()->data.fd);
  }
}

void TcpConnection::HandleWrite() {
  send(channel_.event()->data.fd, write_buffer_, max_buffer_size, 0);
}
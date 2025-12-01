#include "TinyRPC/net/tcp_connection.h"

TcpConnection::TcpConnection(int connect_fd) {
  channel_ = Channel(connect_fd, true, true);
  channel_.set_handle_read(read_task_callback_);
  channel_.set_handle_write(write_task_callback_);
  add_connection_callback_(&channel_);
}

void TcpConnection::set_add_connection_callback(
    std::function<void(Channel*)> callback) {
  add_connection_callback_ = callback;
}

void TcpConnection::set_read_task_callback(std::function<void()> callback) {
  read_task_callback_ = callback;
}

void TcpConnection::set_write_task_callback(std::function<void()> callback) {
  write_task_callback_ = callback;
}
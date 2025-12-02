#include "TinyRPC/net/tcp_connection.h"
#include "TinyRPC/common/console_logger.h"

TcpConnection::TcpConnection(int connect_fd,
                             std::function<void()> read_callback,
                             std::function<void()> write_callback,
                             std::function<void(Channel*)>
                             add_connection_callback) {
  channel_ = Channel(connect_fd, true, true);
  set_work(read_callback, write_callback, add_connection_callback);
  // channel_.set_handle_read(read_task_callback_);
  // channel_.set_handle_write(write_task_callback_);
  // add_connection_callback_(&channel_);
}

// void TcpConnection::set_add_connection_callback(
//     std::function<void(Channel*)> callback) {
//   add_connection_callback_ = callback;
// }

// void TcpConnection::set_read_task_callback(std::function<void()> callback) {
//   read_task_callback_ = callback;
// }
//
// void TcpConnection::set_write_task_callback(std::function<void()> callback) {
//   write_task_callback_ = callback;
// }

void TcpConnection::set_work(std::function<void()> read_callback,
                             std::function<void()> write_callback,
                             std::function<void(Channel*)>
                             add_connection_callback) {

  // read_task_callback_ = read_callback;
  // write_task_callback_ = write_callback;
  channel_.set_handle_read(read_callback);
  channel_.set_handle_write(write_callback);
  this->add_connection_callback_ = add_connection_callback;
  LOG_DEBUG("TcpConnection called add_connection_callback");
  add_connection_callback_(&channel_);
}
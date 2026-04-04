#ifndef PHOTONRPC_TCP_CONNECTION_H
#define PHOTONRPC_TCP_CONNECTION_H

#include "buffer.h"
#include "net/channel.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>

class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  using SendResponse = std::function<void(std::string)>;
  using MessageCallback =
      std::function<void(const std::string&, SendResponse)>;
  using CloseCallback = std::function<void(int)>;

  TcpConnection(int connect_fd, EventLoop* event_loop,
                MessageCallback message_callback,
                CloseCallback close_callback);

  TcpConnection() = delete;

  void ConnectEstablished();

 private:
  void Send(std::string response_data);

  void SendInLoop(std::string response_data);

  void HandleClose();

  Channel channel_;
  EventLoop* event_loop_;

  const int max_buffer_size = 1024;
  Buffer input_buffer_;
  Buffer output_buffer_;
  std::atomic<bool> closed_{false};

  // 注册给epoll的函数
  void HandleRead();

  //注册给epoll的函数
  void HandleWrite();

  MessageCallback message_callback_;
  CloseCallback close_callback_;
};

#endif  //PHOTONRPC_TCP_CONNECTION_H

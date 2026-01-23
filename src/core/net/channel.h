#ifndef PHOTONRPC_CHANNEL_H
#define PHOTONRPC_CHANNEL_H

#include <sys/epoll.h>
#include <functional>

// 该类用于封装具体的文件描述符以及它在epoll中状态
// 是连接上层服务与底层引擎的桥梁
class Channel {
 public:
  // Channel(const int fd, bool read_event, bool write_event);

  Channel(const int fd);

  Channel() {};

  epoll_event* event();

  int fd() const;

  void set_handle_read(std::function<void()> read_callback);

  void set_handle_write(std::function<void()> write_callback);

  void enable_read_event();

  void enable_write_event();

  void disable_read_event();

  void disable_write_event();

  void HandleRead();

  void HandleWrite();

 private:
  epoll_event event_;

  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
};

#endif  //PHOTONRPC_CHANNEL_H
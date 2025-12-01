#ifndef TINYRPC_CHANNEL_H
#define TINYRPC_CHANNEL_H

#include <sys/epoll.h>
#include <functional>

// 该类用于封装具体的文件描述符以及它在epoll中状态
// 是连接上层服务与底层引擎的桥梁
class Channel {
public:
  Channel(const int fd, bool read_event, bool write_event);

  Channel() {};

  // int fd_; // 放到epoll中进行监听的文件描述符
  // unsigned int events_; // 用于记录该channel应当监听的类型
  // unsigned int return_events_; // 实际可执行的事件类型, 由Poller来修改
  epoll_event event_;

  // void HandleEvent();

  void set_handle_read(std::function<void()> read_callback);

  void set_handle_write(std::function<void()> write_callback);

  void read_callback();

  void write_callback();

private:
  std::function<void()> handle_read_;
  std::function<void()> handle_write_;
};


#endif //TINYRPC_CHANNEL_H
#ifndef TINYRPC_POLLER_H
#define TINYRPC_POLLER_H

#define MAX_EVENT_NUMBER 16

#include "TinyRPC/net/channel.h"
#include <sys/epoll.h>

#include <map>
#include <vector>

class Poller {
public:
  Poller();

  int poll(int timeout);

  void register_channel(Channel* channel);
  epoll_event* get_return_events();
  Channel* get_channel_by_fd(int fd);

private:
  // std::vector<epoll_event> return_events_;
  epoll_event return_events_[MAX_EVENT_NUMBER];
  std::map<int, Channel*> fd_channel_map_;
  int epoll_fd_;

};


#endif //TINYRPC_POLLER_H
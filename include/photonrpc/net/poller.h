#ifndef PHOTONRPC_POLLER_H
#define PHOTONRPC_POLLER_H

#define MAX_EVENT_NUMBER 16

#include <sys/epoll.h>
#include "photonrpc/net/channel.h"

#include <map>
#include <vector>

class Poller {
 public:
  Poller();

  int poll(int timeout);

  void RegisterChannel(Channel* channel);

  void RemoveChannel(Channel* channel);

  epoll_event* get_return_events();

  Channel* get_channel_by_fd(int fd);

 private:
  // std::vector<epoll_event> return_events_;
  epoll_event return_events_[MAX_EVENT_NUMBER];
  std::map<int, Channel*> fd_channel_map_;
  int epoll_fd_;
};

#endif  //PHOTONRPC_POLLER_H
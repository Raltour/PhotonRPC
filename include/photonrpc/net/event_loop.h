#ifndef TINYRPC_EVENT_LOOP_H
#define TINYRPC_EVENT_LOOP_H

#include "photonrpc/net/poller.h"

class EventLoop {
 public:
  EventLoop();

  void Loop();

  void AddChannel(Channel* channel);

  void RemoveChannel(Channel* channel);

 private:
  Poller poller_;
};

#endif  //TINYRPC_EVENT_LOOP_H
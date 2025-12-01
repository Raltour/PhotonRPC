#ifndef TINYRPC_EVENT_LOOP_H
#define TINYRPC_EVENT_LOOP_H

#include "TinyRPC/net/poller.h"

class EventLoop {
public:
  EventLoop();

  void Loop();

  void AddChannel(Channel* channel);

private:
  Poller poller_;

};


#endif //TINYRPC_EVENT_LOOP_H
#ifndef PHOTONRPC_EVENT_LOOP_H
#define PHOTONRPC_EVENT_LOOP_H

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

#endif  //PHOTONRPC_EVENT_LOOP_H
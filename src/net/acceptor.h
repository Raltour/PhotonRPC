#ifndef PHOTONRPC_ACCEPTOR_H
#define PHOTONRPC_ACCEPTOR_H

#include "event_loop.h"

class Acceptor {
 public:
  Acceptor();

  void StartListen();

  ~Acceptor();

  void set_new_connection_callback(std::function<void(int)> callback);

  void set_start_listen_callback(std::function<void(Channel*)> callback);

 private:
  int listenfd_;
  Channel listen_channel;

  std::function<void(Channel*)> start_listen_callback_;
  std::function<void(int)> new_connection_callback_;
};

#endif  //PHOTONRPC_ACCEPTOR_H
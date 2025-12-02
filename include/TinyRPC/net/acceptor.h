#ifndef TINYRPC_ACCEPTOR_H
#define TINYRPC_ACCEPTOR_H

#include "TinyRPC/net/event_loop.h"

class Acceptor {
public:
  // Acceptor(){};

  Acceptor(std::function<void(Channel*)>, std::function<void(int)>);

  Acceptor() = delete;

  // void set_new_connection_callback(std::function<void(int)> callback);

  // void set_start_listen_callback(std::function<void(Channel*)> callback);

  // void listen_loop();

private:
  int listenfd_;
  Channel listen_channel;

  std::function<void(Channel*)> start_listen_callback_;
  std::function<void(int)> new_connection_callback_;
};


#endif //TINYRPC_ACCEPTOR_H
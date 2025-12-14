#include "TinyRPC/net/tcp_connection.h"
#include "TinyRPC/common/console_logger.h"
#include "TinyRPC/net/codec.h"

#include <unistd.h>

#include <cstring>

TcpConnection::TcpConnection(int connect_fd,
                             std::function<void(std::string&, std::string&)>
                             service,
                             std::function<void(Channel*)>
                             add_connection_callback)
  : service_(service) {
  channel_ = Channel(connect_fd, true, false);
  channel_.set_handle_read([this] {
    LOG_DEBUG("TcpConnection HandleRead called");
    this->HandleRead();
  });
  channel_.set_handle_write([this] {
    LOG_DEBUG("TcpConnection HandleWrite called");
    this->HandleWrite();
  });
  this->add_connection_callback_ = add_connection_callback;
  LOG_DEBUG("TcpConnection called add_connection_callback");
  add_connection_callback_(&channel_);

}

void TcpConnection::set_close_callback(
    std::function<void(Channel*)> close_callback) {
  close_callback_ = close_callback;
}

void TcpConnection::HandleRead() {
  if (input_buffer_.ReceiveFd(channel_.event()->data.fd)) {
    LOG_DEBUG("TcpConnection received data");
    std::string decoded_data;
    while ((decoded_data = Codec::decode(input_buffer_.PeekData(),
                                         input_buffer_.GetSize())).size() > 0) {
      input_buffer_.RetrieveData(decoded_data.size() + 4);
      std::string response_data;
      service_(decoded_data, response_data);
      LOG_DEBUG(response_data);
      std::string encoded_data = Codec::encode(response_data);
      output_buffer_.WriteData(encoded_data, encoded_data.size());
      while (!output_buffer_.SendFd(channel_.event()->data.fd)) {}
      LOG_DEBUG("TcpConnection finish send data");
      decoded_data.clear();
    }
  } else {
    LOG_DEBUG("TcpConnection connection closed");
    close(channel_.event()->data.fd);
  }
}

void TcpConnection::HandleWrite() {
  // send(channel_.event()->data.fd, output_buffer_.PeekData(), max_buffer_size, 0);
}
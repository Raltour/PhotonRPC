#include "photonrpc/net/tcp_connection.h"
#include "photonrpc/common/logger.h"
#include "photonrpc/net/codec.h"

#include <unistd.h>

TcpConnection::TcpConnection(
    int connect_fd, std::function<void(std::string&, std::string&)> service,
    std::function<void(Channel*)> add_connection_callback)
    : service_(service) {
  channel_ = Channel(connect_fd, true, false);
  channel_.set_handle_read([this] { this->HandleRead(); });
  channel_.set_handle_write([this] { this->HandleWrite(); });
  this->add_connection_callback_ = add_connection_callback;
  add_connection_callback_(&channel_);
}

void TcpConnection::set_close_callback(
    std::function<void(Channel*)> close_callback) {
  close_callback_ = close_callback;
}

void TcpConnection::HandleRead() {
  if (input_buffer_.ReceiveFd(channel_.event()->data.fd)) {
    std::string decoded_data;
    while ((decoded_data = Codec::decode(input_buffer_.PeekData(),
                                         input_buffer_.GetSize()))
               .size() > 0) {
      // TODO: Remove the magic number of 4 here.
      input_buffer_.RetrieveData(decoded_data.size() + 4);

      std::string response_data;
      service_(decoded_data, response_data);

      std::string encoded_data = Codec::encode(response_data);
      output_buffer_.WriteData(encoded_data, encoded_data.size());
      // TODO: Improve the performance here.
      while (!output_buffer_.SendFd(channel_.event()->data.fd)) {}
      decoded_data.clear();
    }
  } else {
    LOG_INFO("TcpConnection(fd:{}) closed", static_cast<int>(channel_.event()->data.fd));
    close(channel_.event()->data.fd);
  }
}

// TODO:: Take good use of reactor write to handle the case when the socket couldn't write right now.
void TcpConnection::HandleWrite() {
  // send(channel_.event()->data.fd, output_buffer_.PeekData(), max_buffer_size, 0);
}
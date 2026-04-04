#include "tcp_connection.h"

#include "../common/logger.h"
#include "codec.h"
#include "event_loop.h"

#include <chrono>
#include <memory>
#include <utility>
#include <unistd.h>

TcpConnection::TcpConnection(int connect_fd, EventLoop* event_loop,
                             MessageCallback message_callback,
                             CloseCallback close_callback)
    : channel_(connect_fd),
      event_loop_(event_loop),
      message_callback_(std::move(message_callback)),
      close_callback_(std::move(close_callback)) {
  channel_.enable_read_event();
  channel_.disable_write_event();
}

void TcpConnection::ConnectEstablished() {
  std::weak_ptr<TcpConnection> weak_self = weak_from_this();
  channel_.set_handle_read([weak_self] {
    if (auto self = weak_self.lock()) {
      self->HandleRead();
    }
  });
  channel_.set_handle_write([weak_self] {
    if (auto self = weak_self.lock()) {
      self->HandleWrite();
    }
  });
  event_loop_->AddChannel(&channel_);
}

void TcpConnection::Send(std::string response_data) {
  if (closed_.load(std::memory_order_acquire)) {
    return;
  }

  auto self = shared_from_this();
  event_loop_->RunInLoop([self, response_data = std::move(response_data)]() mutable {
    self->SendInLoop(std::move(response_data));
  });
}

void TcpConnection::SendInLoop(std::string response_data) {
  if (closed_.load(std::memory_order_acquire)) {
    return;
  }

  std::string encoded_data = Codec::encode(response_data);
  output_buffer_.WriteData(encoded_data, encoded_data.size());
  output_buffer_.SendFd(channel_.fd());
  if (output_buffer_.GetSize() > 0) {
    channel_.enable_write_event();
    event_loop_->UpdateChannel(&channel_);
  }
}

void TcpConnection::HandleClose() {
  auto self = shared_from_this();
  (void)self;
  if (closed_.exchange(true, std::memory_order_acq_rel)) {
    return;
  }

  LOG_INFO("TcpConnection(fd:{}) closed", channel_.fd());
  event_loop_->RemoveChannel(&channel_);
  close(channel_.fd());
  if (close_callback_) {
    close_callback_(channel_.fd());
  }
}

void TcpConnection::HandleRead() {
  auto self = shared_from_this();
  (void)self;
  if (input_buffer_.ReceiveFd(channel_.fd())) {
    std::string decoded_data;
    while ((decoded_data = Codec::decode(input_buffer_.PeekData(),
                                         input_buffer_.GetSize()))
               .size() > 0) {
      // TODO: Remove the magic number of 4 here.
      input_buffer_.RetrieveData(decoded_data.size() + 4);
      std::weak_ptr<TcpConnection> weak_self = weak_from_this();
      message_callback_(decoded_data,
                        [weak_self](std::string response_data) mutable {
                          if (auto locked_self = weak_self.lock()) {
                            locked_self->Send(std::move(response_data));
                          }
                        });
      if (closed_.load(std::memory_order_acquire)) {
        break;
      }
      decoded_data.clear();
    }
  } else {
    HandleClose();
  }
}

void TcpConnection::HandleWrite() {
  auto self = shared_from_this();
  (void)self;
  if (closed_.load(std::memory_order_acquire)) {
    return;
  }

  output_buffer_.SendFd(channel_.fd());
  if (output_buffer_.GetSize() == 0) {
    channel_.disable_write_event();
    event_loop_->UpdateChannel(&channel_);
  }
}

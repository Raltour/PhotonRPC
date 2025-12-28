#ifndef PHOTONRPC_BUFFER_H
#define PHOTONRPC_BUFFER_H

#include <memory>
#include <vector>

class Buffer {
 public:
  Buffer();

  void WriteData(std::string& data, int size);

  std::string PeekData() const;

  // Retrieve data from read_index_ to read_index + size.
  std::string RetrieveData(int size);

  bool ReceiveFd(int fd);

  // Send as much data as possible.
  bool SendFd(int fd);

  int GetSize() const;


private:
  int read_index_;
  int write_index_;
  int data_size_;
  std::unique_ptr<std::vector<char>> buffer_;
};

#endif  //PHOTONRPC_BUFFER_H
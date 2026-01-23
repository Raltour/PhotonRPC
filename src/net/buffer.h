#ifndef PHOTONRPC_BUFFER_H
#define PHOTONRPC_BUFFER_H

#include <memory>
#include <vector>
#include <string>

class Buffer {
 public:
  Buffer();

  Buffer(int init_size);

  void WriteData(std::string& data, int size);

  std::string PeekData() const;

  // Retrieve data from read_index_ to read_index + size.
  bool RetrieveData(int size);

  bool ReceiveFd(int fd);

  // Send as much data as possible.
  bool SendFd(int fd);

  int GetSize() const;

  // Public for testing and potential future use
  std::pair<char*, size_t> GetReadableArea1();
  std::pair<char*, size_t> GetReadableArea2();

 private:
  int read_index_;
  int write_index_;
  int data_size_;
  std::unique_ptr<std::vector<char>> buffer_;

  std::pair<char*, size_t> GetWriteableArea1();
  std::pair<char*, size_t> GetWriteableArea2();
};

#endif  //PHOTONRPC_BUFFER_H
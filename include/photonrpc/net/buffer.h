#ifndef TINYRPC_BUFFER_H
#define TINYRPC_BUFFER_H

#include <memory>
#include <vector>

class Buffer {
 public:
  Buffer();

  void WriteData(std::string& data, int size);

  void RetrieveData(int size);

  bool ReceiveFd(int fd);

  bool SendFd(int fd);

  const char* PeekData();

  int GetSize();

 private:
  int read_index_;
  int write_index_;
  std::unique_ptr<std::vector<char>> buffer_;
};

#endif  //TINYRPC_BUFFER_H
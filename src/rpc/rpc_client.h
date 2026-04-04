#include <string>

class RpcClient {
 public:
  RpcClient();

  ~RpcClient();

  void ConnectToServer();

  void SendMessage(const std::string& message);

  bool ReceiveExact(char* buffer, int size);

 private:
  int sockfd_;
};

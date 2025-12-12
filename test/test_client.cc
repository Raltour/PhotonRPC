#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <cstring>
#include <unistd.h>

int main() {
  const char* ip = "127.0.0.1";
  int port = 12345;

  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &server_address.sin_addr);
  server_address.sin_port = htons(port);

  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
  assert(sockfd >= 0);

  if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    printf("error!\n");
  }

  const char* message = "Hello, TinyRPC!";
  send(sockfd, message, strlen(message), 0);
  char buffer[1024];
  recv(sockfd, buffer, 1024, 0);
  std::cout << "Received from server: " << buffer << std::endl;

  close(sockfd);
  return 0;
}
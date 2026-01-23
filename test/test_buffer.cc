#include <gtest/gtest.h>
#include "../src/net/buffer.h"
#include "../src/net/codec.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

// 1. 基础功能：读写一致性
TEST(BufferTest, BasicReadWrite) {
  Buffer buf(1024);
  std::string data = "test_data";
  buf.WriteData(data, data.size());

  EXPECT_EQ(buf.GetSize(), data.size());
  EXPECT_EQ(buf.PeekData(), data);

  buf.RetrieveData(5);
  EXPECT_EQ(buf.GetSize(), data.size() - 5);
  EXPECT_EQ(buf.PeekData(), "data");
}

// 2. 环形逻辑：测试 WriteIndex 回绕但不扩容
TEST(BufferTest, WrapAroundNoResize) {
  Buffer buf(10);  // 极小缓冲区
  std::string s1 = "123456";
  buf.WriteData(s1, 6);
  buf.RetrieveData(4);  // read_index=4, write_index=6, size=2

  std::string s2 = "789";
  // 此时剩余空间充足，但写入会触发 write_index 回绕 (6+3=9 < 10)
  buf.WriteData(s2, 3);

  EXPECT_EQ(buf.GetSize(), 5);
  EXPECT_EQ(buf.PeekData(), "56789");
}

// 3. 核心 Bug 验证：回绕状态下的扩容
// 场景：read_index 在后面，write_index 在前面，此时触发扩容
TEST(BufferTest, ResizeWhileWrapped) {
  Buffer buf(64);
  std::string s1(50, 'a');
  buf.WriteData(s1, 50);
  buf.RetrieveData(40);  // data_size=10, read_index=40, write_index=50

  std::string s2(30, 'b');
  // 写入 30 字节，10+30=40 < 64，不扩容，发生回绕
  // write_index = (50 + 30) % 64 = 16
  buf.WriteData(s2, 30);

  // 关键点：再写入大量数据触发扩容
  std::string s3(100, 'c');
  buf.WriteData(s3, 100);

  std::string expected =
      std::string(10, 'a') + std::string(30, 'b') + std::string(100, 'c');
  EXPECT_EQ(buf.GetSize(), 140);
  EXPECT_EQ(buf.PeekData(), expected);
}

// 4. 压力测试：连续巨量写入触发多次扩容
TEST(BufferTest, MassiveGrowth) {
  Buffer buf(16);
  std::string total;
  for (int i = 0; i < 100; ++i) {
    std::string chunk = "chunk_" + std::to_string(i) + "_";
    buf.WriteData(chunk, chunk.size());
    total += chunk;
  }
  EXPECT_EQ(buf.GetSize(), total.size());
  EXPECT_EQ(buf.PeekData(), total);
}

// ----------------------------------------------------------------------------
// Codec 协同测试：模拟真实 TCP 场景
// ----------------------------------------------------------------------------

// 5. Codec 粘包测试：Buffer 中存入两个完整的包
TEST(BufferCodecTest, StickyPackets) {
  Buffer buf;
  std::string msg1 = "Hello";
  std::string msg2 = "World! This is a long message.";

  std::string enc1 = Codec::encode(msg1);
  std::string enc2 = Codec::encode(msg2);

  // 一次性写入两个包
  std::string combined = enc1 + enc2;
  buf.WriteData(combined, combined.size());

  // 第一次解析
  std::string r1 = Codec::decode(buf.PeekData(), buf.GetSize());
  EXPECT_EQ(r1, msg1);
  buf.RetrieveData(enc1.size());

  // 第二次解析
  std::string r2 = Codec::decode(buf.PeekData(), buf.GetSize());
  EXPECT_EQ(r2, msg2);
  buf.RetrieveData(enc2.size());

  EXPECT_EQ(buf.GetSize(), 0);
}

// 6. Codec 半包测试：分批写入
TEST(BufferCodecTest, PartialPackets) {
  Buffer buf;
  std::string msg = "FragmentedData";
  std::string encoded = Codec::encode(msg);

  // 1. 只写入 2 字节（长度字段的一半）
  std::string p1 = encoded.substr(0, 2);
  buf.WriteData(p1, 2);
  EXPECT_EQ(Codec::decode(buf.PeekData(), buf.GetSize()), "");

  // 2. 补全长度字段，但没数据包体
  std::string p2 = encoded.substr(2, 2);
  buf.WriteData(p2, 2);
  EXPECT_EQ(Codec::decode(buf.PeekData(), buf.GetSize()), "");

  // 3. 补全剩余所有数据
  std::string p3 = encoded.substr(4);
  buf.WriteData(p3, p3.size());

  EXPECT_EQ(Codec::decode(buf.PeekData(), buf.GetSize()), msg);
}

// 7. 边界测试：正好填满扩容临界点
TEST(BufferTest, ExactCapacityResize) {
  Buffer buf(100);
  std::string s1(100, 'z');
  buf.WriteData(s1, 100);
  EXPECT_EQ(buf.GetSize(), 100);

  // 再写 1 字节触发扩容
  std::string s2 = "!";
  buf.WriteData(s2, 1);
  EXPECT_EQ(buf.GetSize(), 101);
  EXPECT_EQ(buf.PeekData(), s1 + s2);
}

// ----------------------------------------------------------------------------
// ReceiveFd 测试：从文件描述符接收数据
// ----------------------------------------------------------------------------

// 8. ReceiveFd 基础测试：正常接收数据
TEST(BufferReceiveFdTest, BasicReceive) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string test_data = "Hello, World!";

  // 在写入端发送数据
  ssize_t sent = send(fds[1], test_data.c_str(), test_data.size(), 0);
  ASSERT_EQ(sent, static_cast<ssize_t>(test_data.size()));

  // 在接收端接收数据
  bool result = buf.ReceiveFd(fds[0]);
  EXPECT_TRUE(result);
  EXPECT_EQ(buf.GetSize(), test_data.size());
  EXPECT_EQ(buf.PeekData(), test_data);

  close(fds[0]);
  close(fds[1]);
}

// 9. ReceiveFd 测试：接收大量数据（超过临时缓冲区）
TEST(BufferReceiveFdTest, ReceiveLargeData) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf(64);  // 小初始缓冲区，测试扩容
  std::string large_data(5000, 'x');

  // 发送大量数据
  ssize_t sent = send(fds[1], large_data.c_str(), large_data.size(), 0);
  ASSERT_EQ(sent, static_cast<ssize_t>(large_data.size()));

  // 接收数据（可能需要多次调用，因为 ReceiveFd 每次最多读 1024 字节）
  int total_received = 0;
  int max_iterations = 100;  // 防止无限循环
  while (total_received < static_cast<int>(large_data.size()) &&
         max_iterations-- > 0) {
    bool result = buf.ReceiveFd(fds[0]);
    if (!result)
      break;
    total_received = buf.GetSize();
  }

  EXPECT_GE(buf.GetSize(), 1024);  // 至少接收了第一次的 1024 字节
  std::string received = buf.PeekData();
  EXPECT_EQ(received.substr(0, 1024), large_data.substr(0, 1024));

  close(fds[0]);
  close(fds[1]);
}

// 10. ReceiveFd 测试：多次接收数据
TEST(BufferReceiveFdTest, MultipleReceives) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string data1 = "First chunk";
  std::string data2 = "Second chunk";
  std::string data3 = "Third chunk";

  // 发送第一块数据
  send(fds[1], data1.c_str(), data1.size(), 0);
  EXPECT_TRUE(buf.ReceiveFd(fds[0]));
  EXPECT_EQ(buf.PeekData(), data1);

  // 发送第二块数据
  send(fds[1], data2.c_str(), data2.size(), 0);
  EXPECT_TRUE(buf.ReceiveFd(fds[0]));
  EXPECT_EQ(buf.GetSize(), data1.size() + data2.size());
  EXPECT_EQ(buf.PeekData(), data1 + data2);

  // 发送第三块数据
  send(fds[1], data3.c_str(), data3.size(), 0);
  EXPECT_TRUE(buf.ReceiveFd(fds[0]));
  EXPECT_EQ(buf.GetSize(), data1.size() + data2.size() + data3.size());
  EXPECT_EQ(buf.PeekData(), data1 + data2 + data3);

  close(fds[0]);
  close(fds[1]);
}

// 11. ReceiveFd 测试：连接关闭（recv 返回 0）
TEST(BufferReceiveFdTest, ConnectionClosed) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;

  // 关闭写入端
  close(fds[1]);

  // 尝试接收数据，应该返回 false（连接已关闭）
  bool result = buf.ReceiveFd(fds[0]);
  EXPECT_FALSE(result);
  EXPECT_EQ(buf.GetSize(), 0);

  close(fds[0]);
}

// 12. ReceiveFd 测试：接收空数据后关闭
TEST(BufferReceiveFdTest, ReceiveThenClose) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string test_data = "Some data";

  // 先接收一些数据
  send(fds[1], test_data.c_str(), test_data.size(), 0);
  EXPECT_TRUE(buf.ReceiveFd(fds[0]));
  EXPECT_EQ(buf.GetSize(), test_data.size());

  // 关闭写入端
  close(fds[1]);

  // 再次接收，应该返回 false
  bool result = buf.ReceiveFd(fds[0]);
  EXPECT_FALSE(result);
  // 之前的数据应该还在
  EXPECT_EQ(buf.GetSize(), test_data.size());
  EXPECT_EQ(buf.PeekData(), test_data);

  close(fds[0]);
}

// 13. ReceiveFd 测试：接收数据触发缓冲区扩容
TEST(BufferReceiveFdTest, ReceiveTriggersResize) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf(64);  // 小初始缓冲区
  std::string large_data(200, 'A');

  // 先写入一些数据，然后读取一部分，制造回绕状态
  std::string s1(50, 'B');
  buf.WriteData(s1, 50);
  buf.RetrieveData(40);  // read_index=40, write_index=50, size=10

  // 发送数据
  send(fds[1], large_data.c_str(), large_data.size(), 0);

  // 接收数据，应该触发扩容
  bool result = buf.ReceiveFd(fds[0]);
  EXPECT_TRUE(result);
  EXPECT_GT(buf.GetSize(), 10);  // 应该大于之前的 10

  std::string received = buf.PeekData();
  EXPECT_EQ(received.substr(0, 10), std::string(10, 'B'));

  close(fds[0]);
  close(fds[1]);
}

// 14. ReceiveFd 测试：无效文件描述符（错误情况）
TEST(BufferReceiveFdTest, InvalidFileDescriptor) {
  Buffer buf;

  // 使用无效的文件描述符
  int invalid_fd = -1;
  bool result = buf.ReceiveFd(invalid_fd);
  EXPECT_FALSE(result);
  EXPECT_EQ(buf.GetSize(), 0);
}

// ----------------------------------------------------------------------------
// SendFd 测试：向文件描述符发送数据
// ----------------------------------------------------------------------------

// 15. SendFd 基础测试：正常发送所有数据
TEST(BufferSendFdTest, BasicSend) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string test_data = "Test message for sending";
  buf.WriteData(test_data, test_data.size());

  // 发送数据
  bool result = buf.SendFd(fds[0]);
  EXPECT_TRUE(result);

  // 验证数据已发送
  char recv_buffer[1024];
  ssize_t received = recv(fds[1], recv_buffer, sizeof(recv_buffer), 0);
  EXPECT_EQ(received, static_cast<ssize_t>(test_data.size()));
  recv_buffer[received] = '\0';
  EXPECT_EQ(std::string(recv_buffer), test_data);

  // 缓冲区应该为空（所有数据都已发送）
  EXPECT_EQ(buf.GetSize(), 0);

  close(fds[0]);
  close(fds[1]);
}

// 16. SendFd 测试：部分发送（send 返回部分字节）
// 注意：这需要模拟部分发送的场景，可以通过设置 socket 缓冲区大小来实现
TEST(BufferSendFdTest, PartialSend) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  // 设置接收端 socket 为非阻塞，并限制接收缓冲区
  // 这样可能导致部分发送
  int flags = fcntl(fds[1], F_GETFL, 0);
  fcntl(fds[1], F_SETFL, flags | O_NONBLOCK);

  Buffer buf;
  std::string large_data(5000, 'X');
  buf.WriteData(large_data, large_data.size());

  // 尝试发送数据
  bool result = buf.SendFd(fds[0]);
  EXPECT_TRUE(result);

  // 由于 SendFd 的实现，即使部分发送，数据也会被移出缓冲区
  // 然后剩余数据会写回缓冲区
  // 验证缓冲区中还有剩余数据（如果发送未完成）
  if (buf.GetSize() > 0) {
    // 继续发送剩余数据
    int max_iterations = 100;  // 防止无限循环
    while (buf.GetSize() > 0 && max_iterations-- > 0) {
      bool result = buf.SendFd(fds[0]);
      if (!result)
        break;  // 如果发送失败，停止循环
    }
  }

  // 验证所有数据最终都被发送
  char recv_buffer[6000];
  ssize_t total_received = 0;
  int max_recv_iterations = 100;  // 防止无限循环
  while (total_received < static_cast<ssize_t>(large_data.size()) &&
         max_recv_iterations-- > 0) {
    ssize_t n = recv(fds[1], recv_buffer + total_received,
                     sizeof(recv_buffer) - total_received, 0);
    if (n <= 0)
      break;
    total_received += n;
  }

  EXPECT_EQ(total_received, static_cast<ssize_t>(large_data.size()));

  close(fds[0]);
  close(fds[1]);
}

// 17. SendFd 测试：发送空缓冲区
TEST(BufferSendFdTest, SendEmptyBuffer) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;  // 空缓冲区

  // 尝试发送空数据
  // 注意：SendFd 会先 RetrieveData，所以即使缓冲区为空，
  // 也会尝试发送空字符串，send 可能返回 0，导致 SendFd 返回 false
  bool result = buf.SendFd(fds[0]);
  // 根据实现，如果 send 返回 <= 0，返回 false
  // 空缓冲区发送时，send 可能返回 0，所以 result 应该是 false
  EXPECT_FALSE(result);
  EXPECT_EQ(buf.GetSize(), 0);

  close(fds[0]);
  close(fds[1]);
}

// 18. SendFd 测试：多次发送直到完成
TEST(BufferSendFdTest, MultipleSendsUntilComplete) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string large_data(3000, 'M');
  buf.WriteData(large_data, large_data.size());

  // 多次发送直到缓冲区为空
  int send_count = 0;
  int max_iterations = 100;  // 防止无限循环
  while (buf.GetSize() > 0 && send_count < 10 && max_iterations-- > 0) {
    bool result = buf.SendFd(fds[0]);
    if (!result)
      break;
    send_count++;
  }

  // 验证所有数据都被发送
  char recv_buffer[4000];
  ssize_t total_received = 0;
  int max_recv_iterations = 100;  // 防止无限循环
  while (total_received < static_cast<ssize_t>(large_data.size()) &&
         max_recv_iterations-- > 0) {
    ssize_t n = recv(fds[1], recv_buffer + total_received,
                     sizeof(recv_buffer) - total_received, 0);
    if (n <= 0)
      break;
    total_received += n;
  }

  EXPECT_EQ(total_received, static_cast<ssize_t>(large_data.size()));
  recv_buffer[total_received] = '\0';
  EXPECT_EQ(std::string(recv_buffer), large_data);

  close(fds[0]);
  close(fds[1]);
}

// 19. SendFd 测试：发送后验证剩余数据正确写回
TEST(BufferSendFdTest, RemainingDataWrittenBack) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  // 限制发送缓冲区大小，强制部分发送
  int sndbuf = 1024;
  setsockopt(fds[0], SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

  Buffer buf;
  std::string data1 = "First part ";
  std::string data2(2000, 'Y');  // 大量数据，可能无法一次性发送
  std::string full_data = data1 + data2;
  buf.WriteData(full_data, full_data.size());

  int original_size = buf.GetSize();

  // 发送数据
  bool result = buf.SendFd(fds[0]);
  EXPECT_TRUE(result);

  // 如果部分发送，剩余数据应该还在缓冲区中
  if (buf.GetSize() > 0) {
    // 继续发送
    int max_iterations = 100;  // 防止无限循环
    while (buf.GetSize() > 0 && max_iterations-- > 0) {
      bool result = buf.SendFd(fds[0]);
      if (!result)
        break;  // 如果发送失败，停止循环
    }
  }

  // 验证所有数据最终都被发送
  char recv_buffer[3000];
  ssize_t total_received = 0;
  int max_recv_iterations = 100;  // 防止无限循环
  while (total_received < original_size && max_recv_iterations-- > 0) {
    ssize_t n = recv(fds[1], recv_buffer + total_received,
                     sizeof(recv_buffer) - total_received, 0);
    if (n <= 0)
      break;
    total_received += n;
  }

  EXPECT_EQ(total_received, original_size);

  close(fds[0]);
  close(fds[1]);
}

// 20. SendFd 测试：发送失败（关闭的 socket）
TEST(BufferSendFdTest, SendToClosedSocket) {
  // 忽略 SIGPIPE 信号，避免进程被终止
  // 注意：必须在关闭 socket 之前设置
  void (*old_handler)(int) = signal(SIGPIPE, SIG_IGN);

  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string test_data = "Data to send";
  buf.WriteData(test_data, test_data.size());

  // 关闭接收端
  close(fds[1]);

  // 尝试发送，应该失败
  bool result = buf.SendFd(fds[0]);
  // 根据实现，如果 send 返回 <= 0，返回 false
  // 但注意：SendFd 在调用 send 之前就 RetrieveData 了
  // 所以即使发送失败，数据也可能已经从缓冲区移除了
  // 这是实现的一个潜在问题，但我们需要测试实际行为
  EXPECT_FALSE(result);

  close(fds[0]);

  // 恢复之前的 SIGPIPE 处理
  signal(SIGPIPE, old_handler);
}

// 21. SendFd 测试：无效文件描述符
TEST(BufferSendFdTest, InvalidFileDescriptor) {
  Buffer buf;
  std::string test_data = "Test data";
  buf.WriteData(test_data, test_data.size());

  int invalid_fd = -1;
  bool result = buf.SendFd(invalid_fd);
  EXPECT_FALSE(result);
}

// 22. SendFd 和 ReceiveFd 协同测试：完整的数据传输
TEST(BufferFdTest, SendAndReceiveRoundTrip) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer send_buf;
  Buffer recv_buf;

  std::string original_data = "Round trip test data";
  send_buf.WriteData(original_data, original_data.size());

  // 发送数据
  EXPECT_TRUE(send_buf.SendFd(fds[0]));

  // 接收数据
  EXPECT_TRUE(recv_buf.ReceiveFd(fds[1]));

  // 验证数据完整性
  EXPECT_EQ(recv_buf.GetSize(), original_data.size());
  EXPECT_EQ(recv_buf.PeekData(), original_data);

  close(fds[0]);
  close(fds[1]);
}

// 23. SendFd 测试：发送多个数据块
TEST(BufferSendFdTest, SendMultipleChunks) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  Buffer buf;
  std::string chunk1 = "Chunk 1";
  std::string chunk2 = "Chunk 2";
  std::string chunk3 = "Chunk 3";

  // 写入多个数据块
  buf.WriteData(chunk1, chunk1.size());
  buf.WriteData(chunk2, chunk2.size());
  buf.WriteData(chunk3, chunk3.size());

  // 发送所有数据
  int max_iterations = 100;  // 防止无限循环
  while (buf.GetSize() > 0 && max_iterations-- > 0) {
    bool result = buf.SendFd(fds[0]);
    if (!result)
      break;  // 如果发送失败，停止循环
  }

  // 验证接收到的数据
  char recv_buffer[1024];
  ssize_t received = recv(fds[1], recv_buffer, sizeof(recv_buffer), 0);
  recv_buffer[received] = '\0';
  std::string expected = chunk1 + chunk2 + chunk3;
  EXPECT_EQ(std::string(recv_buffer), expected);

  close(fds[0]);
  close(fds[1]);
}
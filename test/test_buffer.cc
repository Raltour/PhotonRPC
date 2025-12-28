#include <gtest/gtest.h>
#include "photonrpc/net/buffer.h"

// 1. 测试基本的读写功能
TEST(BufferTest, AppendAndRetrieve) {
  Buffer buf;

  // 初始状态
  EXPECT_EQ(buf.PeekData(), "");
  EXPECT_EQ(buf.RetrieveData(0), true);
  EXPECT_EQ(buf.RetrieveData(2), false);

  std::string str = "hello photonrpc";
  buf.WriteData(str, str.size());

  EXPECT_EQ(buf.GetSize(), str.size());

  EXPECT_EQ(buf.RetrieveData(6), true);
  std::string read_str = buf.PeekData();

  EXPECT_EQ(read_str, "photonrpc");
}

// 2. 测试自动扩容 (线性增长)
TEST(BufferTest, AutoGrow) {
  Buffer buf(1024);
  std::string long_str(2000, 'x');

  buf.WriteData(long_str, long_str.size());
  // 验证数据完整性
  EXPECT_EQ(buf.PeekData(), long_str);

  std::string long_str2(1000, 'y');
  buf.WriteData(long_str2, long_str2.size());

  EXPECT_EQ(buf.PeekData(), long_str + long_str2);
}

// 3. 测试双指针循环利用内存 (Wrap-around)
// 测试场景：写入数据 -> 读取部分 -> 再次写入直到发生回绕但不扩容
TEST(BufferTest, PointerLoop) {
  Buffer buf(1024);

  // 1. 填充 800 字节
  std::string s1(800, 'a');
  buf.WriteData(s1, s1.size());

  // 2. 读取 600 字节，此时 read_index_ 移到 600
  buf.RetrieveData(600);

  // 3. 再写入 400 字节。此时 800 + 400 = 1200 > 1024，但因为读走了 600，
  // 剩余空间足够，write_index_ 会回绕到数组头部
  std::string s2(400, 'b');
  buf.WriteData(s2, s2.size());

  std::string expected = std::string(200, 'a') + std::string(400, 'b');
  EXPECT_EQ(buf.PeekData(), expected);
}

// 4. 测试在“回绕状态”下触发扩容 (Critical Bug Test)
// 这通常是环形缓冲区最容易出错的地方
TEST(BufferTest, ResizeWhileWrapped) {
  Buffer buf(1024);
  std::string s1(900, 'a');
  buf.WriteData(s1, s1.size());

  // 移动读指针
  buf.RetrieveData(800); // 剩余 100 个 'a'，read_index_ = 800

  // 写入数据使 write_index_ 回绕
  std::string s2(600, 'b');
  buf.WriteData(s2, s2.size()); // 现在总数据 700，write_index_ = (900+600)%1024 = 476

  // 再次写入触发扩容 (700 + 500 > 1024)
  std::string s3(500, 'c');
  buf.WriteData(s3, s3.size());

  std::string expected = std::string(100, 'a') + std::string(600, 'b') + std::string(500, 'c');
  EXPECT_EQ(buf.PeekData(), expected);
}

// 5. 测试连续多次读取和写入
TEST(BufferTest, ContinuousReadWrite) {
  Buffer buf;
  std::string data = "ABCDE";

  for(int i = 0; i < 100; ++i) {
    buf.WriteData(data, data.size());
    EXPECT_EQ(buf.PeekData().substr(0, 5), "ABCDE");
    buf.RetrieveData(data.size());
    EXPECT_EQ(buf.PeekData(), "");
  }
}

// 6. 测试极小缓冲区下的扩容
TEST(BufferTest, SmallInitialSize) {
  Buffer buf(64);
  std::string s1 = "small_buffer_test_with_multiple_resizes";

  // 循环写入并读取，迫使 buffer 在回绕和非回绕状态下多次扩容
  for(int i = 0; i < 10; ++i) {
    buf.WriteData(s1, s1.size());
  }

  std::string full_expected = "";
  for(int i = 0; i < 10; ++i) full_expected += s1;

  EXPECT_EQ(buf.PeekData(), full_expected);
}
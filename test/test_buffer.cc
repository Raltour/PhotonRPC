#include <gtest/gtest.h>
#include <photonrpc/net/buffer.h>

// 1. 测试基本的读写功能
TEST(BufferTest, AppendAndRetrieve) {
  Buffer buf;

  // Initial state
  EXPECT_EQ(buf.PeekData(), "");
  EXPECT_EQ(buf.RetrieveData(0), true);
  EXPECT_EQ(buf.RetrieveData(2), false);

  std::string str = "hello photonrpc";
  buf.WriteData(str, str.size());

  EXPECT_EQ(buf.GetSize(), str.size());

  EXPECT_EQ(buf.RetrieveData(6), true);
  std::string read_str = buf.PeekData();

  EXPECT_EQ(read_str, "photonrpc");
  EXPECT_EQ(buf.GetSize(), str.size() - 6);
}

// // 2. 测试自动扩容 (这是你最担心内存泄漏或越界的地方)
// TEST(BufferTest, AutoGrow) {
//   tinyrpc::Buffer buf;
//   std::string long_str(1000, 'x'); // 1000个x
//
//   buf.append(long_str.c_str(), long_str.size());
//   EXPECT_EQ(buf.readableBytes(), 1000);
//
//   // 再写入大量数据，触发 resize
//   std::string long_str2(2000, 'y');
//   buf.append(long_str2.c_str(), long_str2.size());
//
//   EXPECT_EQ(buf.readableBytes(), 3000);
//   // 这里如果没 crash 且长度对，说明扩容逻辑基本 OK
// }
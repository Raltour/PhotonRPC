#include <gtest/gtest.h>
#include "photonrpc/net/buffer.h"
#include "photonrpc/net/codec.h"

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
  Buffer buf(10); // 极小缓冲区
  std::string s1 = "123456";
  buf.WriteData(s1, 6);
  buf.RetrieveData(4); // read_index=4, write_index=6, size=2

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
  buf.RetrieveData(40); // data_size=10, read_index=40, write_index=50

  std::string s2(30, 'b');
  // 写入 30 字节，10+30=40 < 64，不扩容，发生回绕
  // write_index = (50 + 30) % 64 = 16
  buf.WriteData(s2, 30);

  // 关键点：再写入大量数据触发扩容
  std::string s3(100, 'c');
  buf.WriteData(s3, 100);

  std::string expected = std::string(10, 'a') + std::string(30, 'b') + std::string(100, 'c');
  EXPECT_EQ(buf.GetSize(), 140);
  EXPECT_EQ(buf.PeekData(), expected);
}

// 4. 压力测试：连续巨量写入触发多次扩容
TEST(BufferTest, MassiveGrowth) {
  Buffer buf(16);
  std::string total;
  for(int i = 0; i < 100; ++i) {
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
#include <gtest/gtest.h>
#include "photonrpc/net/codec.h"
#include <string>
#include <vector>

// 辅助函数：手动构造一个带 4 字节头部的原始字节流
std::string ManualEncode(const std::string& payload) {
    int len = payload.size();
    std::string buf;
    buf.append(reinterpret_cast<const char*>(&len), 4);
    buf.append(payload);
    return buf;
}

// ----------------------------------------------------------------------------
// 1. 基础编解码测试
// ----------------------------------------------------------------------------

TEST(CodecTest, BasicEncodeDecode) {
    std::string original = "hello photonrpc";
    std::string encoded = Codec::encode(original);

    // 验证编码长度：4字节头 + 数据长度
    EXPECT_EQ(encoded.size(), original.size() + 4);

    // 验证解码
    std::string decoded = Codec::decode(encoded, encoded.size());
    EXPECT_EQ(decoded, original);
}

TEST(CodecTest, EmptyData) {
    std::string original = "";
    std::string encoded = Codec::encode(original);

    EXPECT_EQ(encoded.size(), 4); // 只有头部

    std::string decoded = Codec::decode(encoded, encoded.size());
    EXPECT_EQ(decoded, "");
}

// ----------------------------------------------------------------------------
// 2. 粘包 (Sticky Packets) 测试
// ----------------------------------------------------------------------------

TEST(CodecTest, StickyPackets) {
    std::string msg1 = "Packet A";
    std::string msg2 = "Packet B is longer";

    std::string buffer = Codec::encode(msg1) + Codec::encode(msg2);

    // 第一次解码：由于当前的 decode 实现逻辑是只处理第一个包
    // 它应该正确提取出第一个包
    std::string decoded1 = Codec::decode(buffer, buffer.size());
    EXPECT_EQ(decoded1, msg1);

    // 模拟应用层逻辑：处理完第一个，截断 buffer 再处理第二个
    std::string remaining = buffer.substr(msg1.size() + 4);
    std::string decoded2 = Codec::decode(remaining, remaining.size());
    EXPECT_EQ(decoded2, msg2);
}

// ----------------------------------------------------------------------------
// 3. 半包 (Half/Partial Packets) 测试 - 这里的测试最为“苛刻”
// ----------------------------------------------------------------------------

TEST(CodecTest, HalfHeader) {
    std::string msg = "critical data";
    std::string encoded = Codec::encode(msg);

    // 场景：头部都没传全 (TCP 只收到了 2 字节)
    // 注意：当前代码 decode 内部直接转 int*，若 size < 4 可能导致越界读取
    // 这里测试是否能优雅返回空
    std::string partial = encoded.substr(0, 2);
    std::string decoded = Codec::decode(partial, partial.size());

    EXPECT_EQ(decoded, "");
}

TEST(CodecTest, HalfPayload) {
    std::string msg = "this payload is incomplete";
    std::string encoded = Codec::encode(msg);

    // 场景：头部完整（4字节），但负载数据只收到了部分
    std::string partial = encoded.substr(0, encoded.size() - 5);
    std::string decoded = Codec::decode(partial, partial.size());

    EXPECT_EQ(decoded, ""); // 数据不足，不应解码出东西
}

// ----------------------------------------------------------------------------
// 4. 边界与特殊字符测试
// ----------------------------------------------------------------------------

TEST(CodecTest, BinaryData) {
    // 测试包含 \0, \n, 以及非 ASCII 字符的数据
    std::string binary;
    binary.push_back('\0');
    binary.push_back('\x01');
    binary.push_back('\xFF');
    binary.append("Mixed Content");

    std::string encoded = Codec::encode(binary);
    std::string decoded = Codec::decode(encoded, encoded.size());

    EXPECT_EQ(decoded.size(), binary.size());
    EXPECT_EQ(decoded, binary);
}

TEST(CodecTest, LargePayload) {
    // 测试 1MB 的大包
    std::string large(1024 * 1024, 'X');
    std::string encoded = Codec::encode(large);
    std::string decoded = Codec::decode(encoded, encoded.size());

    EXPECT_EQ(decoded, large);
}

// ----------------------------------------------------------------------------
// 5. 健壮性测试 (Robustness)
// ----------------------------------------------------------------------------

TEST(CodecTest, IncorrectSizeParam) {
    std::string msg = "data";
    std::string encoded = Codec::encode(msg);

    // 故意传入比实际 buffer 小的 size
    std::string decoded = Codec::decode(encoded, 2);
    EXPECT_EQ(decoded, "");
}

// ----------------------------------------------------------------------------
// 6. 内存越界风险测试 (OOB Read)
// ----------------------------------------------------------------------------
TEST(CodecTest, OutOfBoundsReadRisk) {
  // 场景：网络只传来了 1 个字节的数据
  std::string partial_data = "A";

  // 按照目前的实现，这里会尝试读取 data.data() 开始的 4 个字节
  // 这在 C++ 中是未定义行为 (Undefined Behavior)
  std::string result = Codec::decode(partial_data, 1);

  EXPECT_TRUE(result.empty());
}

// ----------------------------------------------------------------------------
// 7. 伪造/异常巨大的长度测试 (Security)
// ----------------------------------------------------------------------------
TEST(CodecTest, GiantHeaderAttack) {
  // 手工构造一个包：头部声称长度为 1000000，但实际后面只有 "abc"
  int fake_large_size = 1000000;
  std::string malicious_data;
  malicious_data.append(reinterpret_cast<char*>(&fake_large_size), 4);
  malicious_data.append("abc");

  // 此时 size 参数为 7 (4+3)
  // 预期：data_size(1000000) + 4 > size(7)，应返回空，且不应分配巨量内存
  std::string result = Codec::decode(malicious_data, malicious_data.size());

  EXPECT_EQ(result, "");
}

// ----------------------------------------------------------------------------
// 8. 字节序兼容性测试 (Endianness)
// ----------------------------------------------------------------------------
TEST(CodecTest, EndiannessSimulation) {
  std::string data = "test";
  int original_len = data.size(); // 0x00000004

  // 模拟大端序（网络序）传来的长度字段: [00, 00, 00, 04]
  char big_endian_len[4] = {0, 0, 0, 4};
  std::string received_packet;
  received_packet.append(big_endian_len, 4);
  received_packet.append(data);

  std::string result = Codec::decode(received_packet, received_packet.size());

  // 在小端机器上，原代码会将 [00, 00, 00, 04] 识别为 0x04000000 (即 67,108,864)
  // 此时 result 将会返回空（因为 67MB > 实际长度），导致解码失败
  // 这说明代码不具备跨平台兼容性
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  EXPECT_EQ(result, ""); // 这其实是一个隐患的体现
#endif
}

// ----------------------------------------------------------------------------
// 9. 连续粘包深度测试 (Multi-Packet Stress)
// ----------------------------------------------------------------------------
TEST(CodecTest, ComplexStickyAndPartial) {
  std::string s1 = "Alpha";
  std::string s2 = "Beta";
  std::string s3 = "Gamma";

  // 构造：[Msg1完整] + [Msg2完整] + [Msg3的头部]
  std::string stream = Codec::encode(s1) + Codec::encode(s2);
  std::string s3_encoded = Codec::encode(s3);
  stream += s3_encoded.substr(0, 2); // 只给 Msg3 两个字节的头部

  // 第一次解码：应得到 Alpha
  std::string r1 = Codec::decode(stream, stream.size());
  EXPECT_EQ(r1, s1);

  // 模拟应用层移除已处理数据 (4 字节 Header + 5 字节 Alpha)
  stream.erase(0, 4 + s1.size());

  // 第二次解码：应得到 Beta
  std::string r2 = Codec::decode(stream, stream.size());
  EXPECT_EQ(r2, s2);

  // 模拟应用层再次移除
  stream.erase(0, 4 + s2.size());

  // 第三次解码：此时 stream 只有 2 字节，应返回空
  std::string r3 = Codec::decode(stream, stream.size());
  EXPECT_TRUE(r3.empty());
}

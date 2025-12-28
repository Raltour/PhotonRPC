#ifndef TINYRPC_CODEC_H
#define TINYRPC_CODEC_H

#include <string>

class Codec {
 public:
  static std::string decode(const char* data, int size);

  static std::string encode(std::string& data);
};

inline std::string Codec::decode(const char* data, int size) {
  const int* data_size_ptr = reinterpret_cast<const int*>(data);
  int data_size = data_size_ptr[0];
  if (data_size + 4 <= size) {
    std::string decoded_data;
    for (int i = 4; i < data_size + 4; i++) {
      // decoded_data.append(&data[i]);
      decoded_data.push_back(data[i]);
    }
    return decoded_data;
  } else {
    return {};
  }
}

inline std::string Codec::encode(std::string& data) {
  std::string size;
  int data_size = data.size();
  char* data_ptr = reinterpret_cast<char*>(&data_size);
  for (int i = 0; i < 4; i++) {
    char byte = data_ptr[i];
    size.append(1, byte);
  }
  return size + data;
}
#endif  //TINYRPC_CODEC_H
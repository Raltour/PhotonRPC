#ifndef PHOTONRPC_CODEC_H
#define PHOTONRPC_CODEC_H

#include <string>

class Codec {
 public:
  static std::string decode(std::string data, int size);

  static std::string encode(std::string& data);
};

inline std::string Codec::decode(std::string data, int size) {
  if (data.size() < 4) {
    return {};
  }

  int data_size;
  memcpy(&data_size, &data[0], 4);
  // const int* data_size_ptr = reinterpret_cast<const int*>(data.data());
  // int data_size = data_size_ptr[0];
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
  std::string size(4, '0');
  int data_size = data.size();
  memcpy(&size[0], &data_size, 4);
  return size + data;
  // std::string size;
  // int data_size = data.size();
  // char* data_ptr = reinterpret_cast<char*>(&data_size);
  // for (int i = 0; i < 4; i++) {
  //   char byte = data_ptr[i];
  //   size.append(1, byte);
  // }
  // return size + data;
}
#endif  //PHOTONRPC_CODEC_H
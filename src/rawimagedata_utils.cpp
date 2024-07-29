
#include "rawimagedata_utils.h"


u_int16_t bit_order_2_bytes(u_char *s, uint16_t bitorder) {
  if (bitorder == 0x4949) {
    return s[0] | s[1] << 8;
  } else if (bitorder == 0x4D4D) {
    return s[0] << 8 | s[1];
  } else {
    fprintf(stderr, "Invalid Endain Value\n");
    return 0;
  }
}

u_int16_t read_2_bytes(std::ifstream& file, uint16_t bitorder) {
  u_char buffer[2] = {0xff,0xff};
  file.read(reinterpret_cast<char*>(&buffer), 2);
  return bit_order_2_bytes(buffer, bitorder);
}

u_int32_t bit_order_4_bytes(u_char *s, uint16_t bitorder) {
  if (bitorder == 0x4949) {
    return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
  } else if (bitorder == 0x4D4D) {
    return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
  } else {
    fprintf(stderr, "Invalid Endain Value\n");
    return 0;
  }
}

u_int32_t read_4_bytes(std::ifstream& file, uint16_t bitorder) {
  u_char buffer[4] = {0xff,0xff,0xff,0xff};
  file.read(reinterpret_cast<char*>(&buffer), 4);
  return bit_order_4_bytes(buffer, bitorder);
}

double get_file_size(std::ifstream& file) {
  double file_size = 0;
  file.seekg(0, std::ios::end);
  file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  return file_size;
}
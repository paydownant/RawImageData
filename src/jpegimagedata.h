
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <cassert>
#include <arpa/inet.h>

const u_char MAGIC_JPEG[2] = {0xff, 0xd8};
const u_char ZZ_MATRIX[] = {
  0,  1,  8,  16, 9,  2,  3,  10,
  17, 24, 32, 25, 18, 11, 4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6,  7,  14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46, 
  53, 60, 61, 54, 47, 55, 62, 63
};

struct quantisation_table_t {
  u_int table[64] = {0};
  bool set = false;
};

struct huff_t {
  u_short symbol;
  u_short code;
};

struct jpeg_info_t {
  // APP
  int identifier, ver, dpi;
  // SOF
  int frame_type, precision, width, height, components;
  // DHT
  u_short *huff[20], *free[20];
  huff_t *huff_data[20], *free_data[20];
  // DQT
  quantisation_table_t quant[4];
  // DRI
  int restart;
};

bool jpeg_info(std::ifstream& file, jpeg_info_t* jpeg_info, bool info_only);

void parse_quantisation_table(jpeg_info_t** jpeg_info, const u_char** data, const u_int length);
void parse_huff_table(jpeg_info_t** jpeg_info, const u_char** data, const u_int length);


huff_t* get_huff_tree(const u_char **dht_segment);

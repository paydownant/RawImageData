#ifndef JPEGIMAGEDATA_H
#define JPEGIMAGEDATA_H

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


struct colour_component_t {
  u_int h_sampling_factor = 1;
  u_int v_sampling_factor = 1;
  u_int quantisation_table_id = 0;
  u_int huff_dc_table_id = 0;
  u_int huff_ac_table_id = 0;
  bool set = false;
};

struct quantisation_table_t {
  u_int table[64] = { 0 };
  bool set = false;
};

struct huff_table_t {
  u_int offsets[17] = { 0 };
  u_int symbols[162] = { 0 };
  bool set = false;
};

struct jpeg_info_t {
  // SOF
  u_int frame_type = 0, precision = 0, width = 0, height = 0, components = 0;
  colour_component_t colour_components[6];
  bool zero_based = false;

  // DQT
  quantisation_table_t quant[4];
  
  // DHT
  huff_table_t huff_dc_tables[4];
  huff_table_t huff_ac_tables[4];
  
  // SOS
  u_int start_selection = 0;
  u_int end_selection = 63;
  u_int s_approx_high = 0;
  u_int s_approx_low = 0;
  std::vector<u_int> huff_data;
  
  // DRI
  u_int restart_interval = 0;
};

bool parse_jpeg_info(std::ifstream& file, jpeg_info_t* jpeg_info, bool info_only);
bool parse_sof(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length);
bool parse_dqt(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length);
bool parse_dht(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length);
bool parse_sos(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length);
bool parse_dri(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length);
void print_jpeg_info(jpeg_info_t* jpeg_info);

#endif
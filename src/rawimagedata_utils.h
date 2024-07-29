
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

u_int16_t bit_order_2_bytes(u_char *s, uint16_t bitorder);
u_int16_t read_2_bytes(std::ifstream& file, uint16_t bitorder);

u_int32_t bit_order_4_bytes(u_char *s, uint16_t bitorder);
u_int32_t read_4_bytes(std::ifstream& file, uint16_t bitorder);

double get_file_size(std::ifstream& file);
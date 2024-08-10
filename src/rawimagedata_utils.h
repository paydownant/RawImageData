
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

u_int8_t bit_order_1_byte(u_char *s, uint16_t bitorder);
u_int8_t read_1_byte_unsigned(std::ifstream& file, uint16_t bitorder);
int8_t read_1_byte_signed(std::ifstream& file, uint16_t bitorder);

u_int16_t bit_order_2_bytes(u_char *s, uint16_t bitorder);
u_int16_t read_2_bytes_unsigned(std::ifstream& file, uint16_t bitorder);
int16_t read_2_byte_signed(std::ifstream& file, uint16_t bitorder);

u_int32_t bit_order_4_bytes(u_char *s, uint16_t bitorder);
u_int32_t read_4_bytes_unsigned(std::ifstream& file, uint16_t bitorder);
int32_t read_4_byte_signed(std::ifstream& file, uint16_t bitorder);

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <arpa/inet.h>

class JpegImageData {

public:
  JpegImageData(std::ifstream& file);
  ~JpegImageData();

  bool jpeg_header(std::ifstream& file);

private:
  

public:

private:

  struct jpeg_header_t {
    // APP0
    int length, identifier, ver, dpi, width, height;

  };
  
  const u_char MAGIC_JPEG[2] = {0xff, 0xd8};

};


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

  bool jpeg_info(std::ifstream& file);

private:
  

public:

private:

  struct jpeg_info_t {
    // APP
    int identifier, ver, dpi;
    // SOF
    int precision, width, height, components;
    // DRI
    int restart;

  };
  
  const u_char MAGIC_JPEG[2] = {0xff, 0xd8};

};

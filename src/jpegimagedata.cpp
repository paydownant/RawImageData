
#include "jpegimagedata.h"

JpegImageData :: JpegImageData(std::ifstream& file) {
  
}

JpegImageData :: ~JpegImageData() {

}

bool JpegImageData :: jpeg_header(std::ifstream& file) {
  jpeg_header_t *jpeg_header;
  u_char buffer[2] = {0xFF, 0xFF};
  memset(jpeg_header, 0, sizeof(*jpeg_header));

  if (!file.read(reinterpret_cast<char*>(&buffer), 2)) {
    return false;
  }
  if (memcmp(buffer, MAGIC_JPEG, 2)) {
    // Magic number does not match FF D8 ...
    printf("check jpeg: %02X %02X\n", buffer[0], buffer[1]);
    return false;
  }

  u_char marker_length[4], data[0xfffff];
  u_short marker, length;
  while (file.read(reinterpret_cast<char*>(&marker_length), 4)) {
    if (marker_length[0] != 0xff) {
      continue;
    }
    marker = (marker_length[0] << 8 | marker_length[1]);
    length = (marker_length[2] << 8 | marker_length[3]) - 2;
    if (marker == 0xffda) {
      break;
    }
    file.read(reinterpret_cast<char*>(&data), length);
    switch (marker) {
      case 0xffe0:  // APP0
        break;
      case 0xffc0:  // SOF0
        break;
    
      default:  // skip
        file.seekg(length, std::ios_base::cur);
        break;
    }
  }
  

}

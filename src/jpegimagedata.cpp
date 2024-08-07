
#include "jpegimagedata.h"

JpegImageData :: JpegImageData(std::ifstream& file) {
  
}

JpegImageData :: ~JpegImageData() {

}

bool JpegImageData :: jpeg_info(std::ifstream& file) {
  jpeg_info_t *jpeg_info;
  memset(jpeg_info, 0, sizeof(*jpeg_info));

  u_char buffer[2];  
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
    
    file.read(reinterpret_cast<char*>(&data), length);
    switch (marker) {
      case 0xffe0:  // APP0 (Application 0)
        break;
      case 0xffc0:  // SOF0 (Start of Frame)
        jpeg_info->precision = data[0];
        jpeg_info->height = (data[1] << 8 | data[2]);
        jpeg_info->width = (data[3] << 8 | data[4]);
        jpeg_info->components = data[5];
        break;
      case 0xffc4:  // DHF (Define Huffman Table)
        break;
      case 0xffda:  // SOS (Start of Scan)
        break;
      case 0xffdb:  // DQT (Define Quantisation Table)
        break;
      case 0xffdd:  // DRI (Define Restart Interval)
        jpeg_info->restart = (data[0] << 8 | data[1]);
        break;
    
      default:  // skip
        file.seekg(length, std::ios_base::cur);
        break;
    }
    
    if (marker == 0xffda) {
      break;
    }
  }
  

}

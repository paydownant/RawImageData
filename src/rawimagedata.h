
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

using file_stream_t = std::ifstream;
using string_t = std::string;
using runtime_error_t = std::runtime_error;

class RawImageData {

public:
  /* Public Functions */
  RawImageData(const string_t& file_path);

private:
  /* Private Functions */
  int raw_identify();

  int parse_raw_image(uint32_t raw_image_file_base);
  int parse_raw_image_ifd(uint32_t raw_image_file_base);

  void process_tag(uint32_t raw_image_file_base, int ifd);
  uint64_t get_tag_data_offset(uint32_t ifd_base, uint32_t tag_type, uint32_t tag_count);
  void get_tag_header(uint32_t raw_image_file_base, uint32_t *tag_id, uint32_t *tag_type, uint32_t *tag_count, uint32_t *tag_offset);
  uint32_t get_tag_value_int(uint32_t tag_type);
  void get_time_stamp();

public:
  /* Public Variables */


private:
  /* Private Variables */
  string_t file_path;
  file_stream_t file;

  struct raw_image_file_t {
    uint32_t base;
    uint16_t bitorder;      // Byte order indicator ("II" 0x4949 for little-endian, "MM" 0x4D4D for big-endian)
    uint16_t version;       // Version
    uint32_t first_offset;  // Offset to the first IFD
    uint8_t raw_ifd_count;  // Number of IFD
    double file_size;       // File size
    char make[64];
    char model[64];
    char software[64];
    time_t time_stamp;
    int time_zone;
    char artist[64];
  } raw_image_file;

  struct raw_image_ifd_t {
    int n_tag_entries;
    int width, height, bps, compression, pinterpret, orientation;
    int x_res, y_res, planar_config;
    int strip_offset, sample_pixel, strip_byte_counts, rows_per_strip;
    int tile_width, tile_length;
    float shutter;
  } raw_image_ifd[8];

  enum class Raw_Tag_Type_Bytes {
    BYTE = 1,
    ASCII = 1,
    SHORT = 1,
    LONG = 2,
    RATIONAL = 4,
    SBYTE = 8,
    UNDEFINED = 1,
    SSHORT = 1,
    SLONG = 2,
    SRATIONAL = 4,
    FLOAT = 8,
    DOUBLE = 4,
    IFD = 8,
    LONG8 = 4
  };

  // Constants
  //static constexpr unsigned raw_tag_type_bytes[14] = {1,1,1,2,4,8,1,1,2,4,8,4,8,4};

};
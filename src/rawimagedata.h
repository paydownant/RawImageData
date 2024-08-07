
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <arpa/inet.h>

using file_stream_t = std::ifstream;
using string_t = std::string;
using runtime_error_t = std::runtime_error;

class RawImageData {
public:
  /* Public Variables */

private:
  /* Private Variables */
  string_t file_path;
  file_stream_t file;

  struct raw_image_file_t {
    u_int32_t base;
    u_int16_t bitorder;       // Byte order indicator ("II" 0x4949 for little-endian, "MM" 0x4D4D for big-endian)
    u_int16_t version;        // Version
    
    off_t first_offset;       // Offset to the first IFD
    u_int8_t raw_ifd_count;   // Number of IFD
    double file_size;         // File size
    char make[64];
    char model[64];
    char software[64];
    time_t time_stamp;
    int time_zone;
    char artist[64];
    char copyright[64];

    off_t exif_offset;
  } raw_image_file;

  struct exif_t {
    char camera_make[64];
    char camera_model[64];
    time_t date_time;

    double focal_length;
    double exposure;
    double f_number;
    int iso_sensitivity;

    char lens_model[64];

    off_t icc_profile_offset;
    uint32_t icc_profile_count;

    off_t gps_offset;
    char gps_latitude_reference;
    double gps_latitude;
    char gps_longitude_reference;
    double gps_longitude;
  } exif;

  struct raw_image_ifd_t {
    u_int32_t n_tag_entries;
    int width, height, bps, compression, pinterpret, orientation;
    double x_res, y_res;
    int planar_config;
    off_t strip_offset, jpeg_if_offset, tile_offset;
    int sample_pixel, strip_byte_counts, rows_per_strip, jpeg_if_length;
    int tile_width, tile_length;
  } raw_image_ifd[8];

  struct jpeg_header_t {
    int algo, bits, height, width, clrs, sraw, psv, restart, vpred[6];
    ushort quant[64], idct[64], *huff[20], *free[20], *row;
  };

  enum class Raw_Tag_Type_Bytes {
    BYTE = 1,
    ASCII = 1,
    SHORT = 2,
    LONG = 4,
    RATIONAL = 8,
    SBYTE = 1,
    UNDEFINED = 1,
    SSHORT = 2,
    SLONG = 4,
    SRATIONAL = 8,
    FLOAT = 4,
    DOUBLE = 8
  };

  // Constants
  const u_char MAGIC_JPEG[2] = {0xff, 0xd8};
  //static constexpr unsigned raw_tag_type_bytes[14] = {1,1,1,2,4,8,1,1,2,4,8,4,8,4};
  

public:
  /* Public Functions */
  RawImageData(const string_t& file_path);
  ~RawImageData();

private:
  /* Private Functions */
  bool raw_identify();

  bool parse_raw_image(off_t raw_image_file_base);
  u_int64_t parse_raw_image_ifd(off_t raw_image_file_base);
  void parse_raw_image_tag(off_t raw_image_file_base, u_int64_t ifd);
  bool parse_exif_data(off_t raw_image_file_base);
  bool parse_strip_data(off_t raw_image_file_base, u_int64_t ifd);
  bool parse_gps_data(off_t raw_image_file_base);
  bool parse_time_stamp();

  off_t get_tag_data_offset(off_t raw_image_file_base, u_int32_t tag_type, u_int32_t tag_count);
  void get_tag_header(off_t raw_image_file_base, u_int32_t *tag_id, u_int32_t *tag_type, u_int32_t *tag_count, off_t *tag_offset);
  double get_tag_value(u_int32_t tag_type);

  bool get_jpeg_header(jpeg_header_t* jpeg_header, bool info_only);

};
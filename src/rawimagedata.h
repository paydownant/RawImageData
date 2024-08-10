
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <arpa/inet.h>

#include "rawimagedata_utils.h"
#include "jpegimagedata.h"

using file_stream_t = std::ifstream;
using string_t = std::string;

class RawImageData {
public:
  /* Public Variables */

private:
  /* Private Variables */
  string_t file_path;
  file_stream_t file;

  struct raw_image_file_t {
    u_int32_t base = 0;
    u_int16_t bitorder = 0x4949;  // Byte order indicator ("II" 0x4949 for little-endian, "MM" 0x4D4D for big-endian)
    u_int16_t version = 0;        // Version
    
    u_int8_t raw_ifd_count = 0;   // Number of IFD
    int file_size = 0;         // File size
    char make[64] = { 0 };
    char model[64] = { 0 };
    char software[64] = { 0 };
    time_t time_stamp = 0;
    int time_zone = 0;            // default = UTC
    char artist[64] = { 0 };
    char copyright[64] = { 0 };

    off_t exif_offset = 0;
  } raw_image_file;

  struct exif_t {
    char camera_make[64] = { 0 };
    char camera_model[64] = { 0 };
    time_t date_time = -1;

    double focal_length = -1;
    double exposure = -1;
    double f_number = -1;
    int iso_sensitivity = -1;

    char lens_model[64] = { 0 };

    off_t icc_profile_offset = 0;
    uint32_t icc_profile_count = 0;

    off_t gps_offset = 0;
    char gps_latitude_reference = 0;
    double gps_latitude = 0;
    char gps_longitude_reference = 0;
    double gps_longitude = 0;
  } exif;

  struct raw_image_ifd_t {
    u_int32_t n_tag_entries;
    int width, height, bps, compression, pinterpret, orientation;
    double x_res, y_res;
    int planar_config;
    off_t strip_offset, tile_offset;
    int sample_pixel, strip_byte_counts, rows_per_strip, jpeg_if_length;
    int tile_width, tile_length;
  } raw_image_ifd[8];

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
  //static constexpr unsigned raw_tag_type_bytes[14] = {1,1,1,2,4,8,1,1,2,4,8,4,8,4};
  

public:
  /* Public Functions */
  RawImageData(const string_t& file_path);
  ~RawImageData();

private:
  /* Private Functions */
  bool raw_identify();

  bool parse_raw(off_t raw_image_file_base);

  bool parse_raw_image(off_t raw_image_file_base);
  u_int64_t parse_raw_image_ifd(off_t raw_image_file_base);
  void parse_raw_image_tag(off_t raw_image_file_base, u_int64_t ifd);
  bool parse_exif_data(off_t raw_image_file_base);
  bool parse_strip_data(off_t raw_image_file_base, u_int64_t ifd);
  bool parse_maker_note(off_t raw_image_file_base, int uptag);
  bool parse_gps_data(off_t raw_image_file_base);
  bool parse_time_stamp();

  off_t get_tag_data_offset(off_t raw_image_file_base, u_int32_t tag_type, u_int32_t tag_count);
  void get_tag_header(off_t raw_image_file_base, u_int32_t *tag_id, u_int32_t *tag_type, u_int32_t *tag_count, off_t *tag_offset);
  double get_tag_value(u_int32_t tag_type);

  void print_data();

};

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <arpa/inet.h>

#include "rawimagedata_utils.h"
#include "jpegimagedata.h"


class RawImageData {
  
public:
  /* Public Variables */

private:
  /* Private Variables */
  std::string file_path;
  std::ifstream file;

  struct raw_image_ifd_t {
    u_int n_tag_entries = 0;
    int width = 0, height = 0, bps = 0, compression = 0, pinterpret = 0, orientation = 0;
    double x_res = 0, y_res = 0;
    int planar_config = 0;
    off_t strip_offset = 0, tile_offset = 0;
    int sample_pixel = 0, strip_byte_counts = 0, rows_per_strip = 0, jpeg_if_length = 0;
    int tile_width = 0, tile_length = 0;

    bool set = false;
  };

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
    u_int icc_profile_count = 0;

    off_t gps_offset = 0;
    char gps_latitude_reference = 0;
    double gps_latitude = 0;
    char gps_longitude_reference = 0;
    double gps_longitude = 0;
  };

  struct raw_image_file_t {
    u_int32_t base = 0;
    u_int16_t bitorder = 0x4949;  // Byte order indicator ("II" 0x4949 for little-endian, "MM" 0x4D4D for big-endian)
    u_int16_t version = 0;        // Version
    
    u_int raw_ifd_count = 0;   // Number of IFD
    int file_size = 0;         // File size
    char make[64] = { 0 };
    char model[64] = { 0 };
    char software[64] = { 0 };
    time_t time_stamp = 0;
    int time_zone = 0;            // default = UTC
    char artist[64] = { 0 };
    char copyright[64] = { 0 };

    off_t exif_offset = 0;

    exif_t exif;
    raw_image_ifd_t raw_image_ifd[8];

  } raw_image_file;

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
  RawImageData(const std::string& file_path);
  ~RawImageData();

private:
  /* Private Functions */
  bool raw_identify();

  bool parse_raw(off_t raw_image_file_base);

  bool parse_raw_image(off_t raw_image_file_base);
  bool parse_raw_image_ifd(off_t raw_image_file_base);
  void parse_raw_image_tag(off_t raw_image_file_base, u_int ifd);
  bool parse_exif_data(off_t raw_image_file_base);
  bool parse_strip_data(off_t raw_image_file_base, u_int ifd);
  bool parse_makernote(off_t raw_image_file_base, int uptag);
  void parse_markernote_tags_nikon(off_t raw_image_file_base);
  bool parse_gps_data(off_t raw_image_file_base);
  bool parse_time_stamp();

  off_t get_tag_data_offset(off_t raw_image_file_base, u_int tag_type, u_int tag_count);
  void get_tag_header(off_t raw_image_file_base, u_int *tag_id, u_int *tag_type, u_int *tag_count, off_t *tag_offset);
  double get_tag_value(u_int tag_type);

  void print_data();

};
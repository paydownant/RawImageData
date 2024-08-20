#ifndef RAWIMAGEDATA_H
#define RAWIMAGEDATA_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <arpa/inet.h>

#include "rawimagedata_utils.h"
#include "jpegimagedata.h"

#define COPY_IF_SET(dest, src, field) if (src.field[0] != 0) strcpy(dest.field, src.field)
#define ASSIGN_IF_SET(dest, src, field) if (src.field != 0) dest.field = src.field

class RawImageData {

public:
  /* Public Variables */
protected:
  /* Protected Variables */
  std::string file_path;
  std::ifstream file;

  struct white_balance_multiplier_t {
    bool set = false;
    double r = 0;
    double g = 0;
    double b = 0;
    double alpha = 0;
  };

  struct rggb_t {
    bool set = false;
    u_int r = 0;
    u_int g_r = 0;
    u_int g_b = 0;
    u_int b = 0;
  };

  struct img_frame_t {
    u_int width = 0, height = 0;
    u_int bps = 0;
    u_int compression = 0;
    u_int sample_pixel = 0;
    u_int pinterpret = 0;
    u_int tile_width = 0, tile_length = 0;
    u_int x_res = 0, y_res = 0;
    u_int planar_config;
    int orientation = 0;
  };

  struct raw_util_t {
    u_int cfa = 0;
    white_balance_multiplier_t white_balance_multi_cam;
    rggb_t cblack;
  };

  struct lens_t {
    bool set = false;
    char lens_model[64] = { 0 };
    u_short lens_type = 0;
    double min_focal_length = 0;
    double max_focal_length = 0;
    double min_f_number = 0;
    double max_f_number = 0;
  };

  struct exif_t {
    off_t offset = 0;

    char camera_make[64] = { 0 };
    char camera_model[64] = { 0 };
    char software[64] = { 0 };

    double focal_length = 0;
    double exposure = 0;
    double f_number = 0;
    double iso_sensitivity = 0;

    u_int image_count = 0;
    u_int shutter_count = 0;

    char artist[64] = { 0 };
    char copyright[64] = { 0 };

    off_t icc_profile_offset = 0;
    u_int icc_profile_count = 0;

    off_t gps_offset = 0;
    char gps_latitude_reference = 0;
    double gps_latitude = 0;
    char gps_longitude_reference = 0;
    double gps_longitude = 0;

    time_t date_time = 0;
    char date_time_str[20] = { 0 };

    lens_t lens_info;
  };

  struct thumb_t {
    img_frame_t frame;

    off_t offset = 0;
    u_int length = 0;
  };

  struct raw_data_ifd_t {
    int _id = -1;

    u_int n_tag_entries = 0;

    img_frame_t frame;
    raw_util_t util;
    exif_t exif;
    
    off_t data_offset = 0;
    off_t tile_offset = 0;
    off_t meta_offset = 0;
    
    u_int strip_byte_counts = 0;
    u_int rows_per_strip = 0;
    u_int jpeg_if_length = 0;

  };

  struct raw_data_t {
    u_int32_t base = 0;
    u_int16_t bitorder = 0x4949;  // Byte order indicator ("II" 0x4949 for little-endian, "MM" 0x4D4D for big-endian)
    u_int16_t version = 0;        // Version
    int file_size = 0;            // File size

    u_int ifd_count = 0;          // Total Number of IFD
    raw_data_ifd_t ifds[8];       // IFDs
    raw_data_ifd_t main_ifd;      // Raw IFD

  } raw_data;

private:
  /* Private Variables */
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

  const u_int BIT_MASK = 0x01010101;

  //static constexpr unsigned raw_tag_type_bytes[14] = {1,1,1,2,4,8,1,1,2,4,8,4,8,4};
  

public:
  /* Public Functions */
  RawImageData(const std::string& file_path);
  ~RawImageData();

  bool load_raw();

protected:
  /* Protected Functions */
  bool raw_identify();

  bool apply_raw_data();

  bool parse_raw(off_t raw_data_base);
  bool parse_raw_data(off_t raw_data_base);
  bool parse_raw_data_ifd(off_t raw_data_base);
  void parse_raw_data_ifd_tag(u_int ifd, off_t raw_data_base);
  bool parse_exif_data(u_int ifd, off_t raw_data_base);
  bool parse_strip_data(u_int ifd, off_t raw_data_base);
  bool parse_gps_data(u_int ifd, off_t raw_data_base);
  bool parse_time_stamp(u_int ifd);

  virtual bool parse_makernote(u_int ifd, off_t raw_data_base, int uptag) = 0;
  virtual void parse_markernote_tag(u_int ifd, off_t raw_data_base, int uptag) = 0;

  off_t get_tag_data_offset(off_t raw_data_base, u_int tag_type, u_int tag_count);
  void get_tag_header(off_t raw_data_base, u_int *tag_id, u_int *tag_type, u_int *tag_count, off_t *tag_offset);
  double get_tag_value(u_int tag_type);

  void print_data(bool rawFileData, bool rawTiffIfds);

};

#endif
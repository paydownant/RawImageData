
#include "nikon_raw.h"

NikonRaw :: NikonRaw(const std::string& filepath) : RawImageData(filepath) {}
NikonRaw :: ~NikonRaw(){}

bool NikonRaw :: parse_makernote(off_t raw_data_base, int uptag) {
  
  char maker_magic[10];
  off_t base, offset;
  
  file.read(maker_magic, 10);
  if (strncasecmp(maker_magic, "Nikon", 6)) {
    fprintf(stderr, "ERROR: Makernote Conflict: %s, was given %s\n", raw_data.exif.camera_make, maker_magic);
    return false;
  }
  
  base = file.tellg();
  raw_data.bitorder = read_2_bytes_unsigned(file, raw_data.bitorder);
  read_2_bytes_unsigned(file, raw_data.bitorder);
  offset = read_4_bytes_unsigned(file, raw_data.bitorder);
  if (offset != 8) {
    return false;
  }

  u_int n_tag_entries;
  u_int16_t bitorder;

  bitorder = raw_data.bitorder;
  n_tag_entries = read_2_bytes_unsigned(file, raw_data.bitorder);

  for (u_int i = 0; i < n_tag_entries; ++i) {
    raw_data.bitorder = bitorder;
    parse_markernote_tag(base, uptag);
  }

  return true;
}

/*
 * https://exiv2.org/tags-nikon.html
 */
void NikonRaw :: parse_markernote_tag(off_t raw_data_base, int uptag) {
  u_int tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;
  get_tag_header(raw_data_base, &tag_id, &tag_type, &tag_count, &tag_offset);
  printf("Makernote tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
  tag_data_offset = get_tag_data_offset(raw_data_base, tag_type, tag_count);  
  
  char buffer[16] = { 0 };
  u_int n, serial = 0;
  file.seekg(tag_data_offset, std::ios::beg); // Jump to data offset
  switch (tag_id) {
    case 0x0002:  // Exif.Nikon3.ISOSpeed (First Value: 0, Second Value: ISO Speed)
      if (!raw_data.exif.iso_sensitivity) {
        get_tag_value(tag_type);
        raw_data.exif.iso_sensitivity = get_tag_value(tag_type);
        printf("Makernote ISO Sens: %lf\n", raw_data.exif.iso_sensitivity);
      }
      break;
    case 0x0004:  // Exif.Nikon3.Quality
      break;
    case 0x000c:  // Exif.Nikon3.WB_RBLevels (RBG-)
      raw_data.white_balance_multi_cam.r = get_tag_value(tag_type);
      raw_data.white_balance_multi_cam.b = get_tag_value(tag_type);
      raw_data.white_balance_multi_cam.g = get_tag_value(tag_type);
      break;
    case 0x000d:  // Exif.Nikon3.ProgramShift
      break;
    case 0x0011:  // Exif.Nikon3.Preview
      file.seekg(get_tag_value(tag_type) + raw_data_base, std::ios::beg);
      printf("Parse makernote offset: %d\n", file.tellg());
      parse_raw_data_ifd(raw_data_base);
      break;
    case 0x001d:  // Exif.Nikon3.SerialNumber
      if (tag_count > 16) {
        fprintf(stderr, "ERROR: Makernote 0x001d: tag_count Exceeded 16: %d\n", tag_count);
        break;
      }
      file.read(buffer, tag_count);
      serial = std::stoi(buffer, 0, 10);
      break;
    case 0x003d:  // Exif.Nikon3.CBlack
      if (tag_type == 3 && tag_count == 4) {
        raw_data.cblack.r = (u_short)get_tag_value(tag_type) >> (14 - raw_data.frame.bps);
        raw_data.cblack.g_r = (u_short)get_tag_value(tag_type) >> (14 - raw_data.frame.bps);
        raw_data.cblack.b = (u_short)get_tag_value(tag_type) >> (14 - raw_data.frame.bps);
        raw_data.cblack.g_b = (u_short)get_tag_value(tag_type) >> (14 - raw_data.frame.bps);
      }
      break;
    case 0x0083:  // Exif.Nikon3.LensType (6: Nikon D Series, 12: Nikon G Series)
      raw_data.exif.lens_info.lens_type = get_tag_value(tag_type);
      break;
    case 0x0084:  // Exif.Nikon3.Lens
      raw_data.exif.lens_info.min_focal_length = get_tag_value(tag_type);
      raw_data.exif.lens_info.max_focal_length = get_tag_value(tag_type);
      raw_data.exif.lens_info.min_f_number = get_tag_value(tag_type);
      raw_data.exif.lens_info.max_f_number = get_tag_value(tag_type);
      break;
    case 0x008c:  // Exif.Nikon3.ContrastCurve
    case 0x0096:  // Exif.Nikon3.LinearizationTable
      raw_data.meta_offset = file.tellg();
      printf("Meta Offset: %d\n", raw_data.meta_offset);
      break;
    case 0x0097:  // Exif.Nikon3.ColorBalance
      file.read(buffer, 4);
      n = std::stoi(buffer, 0, 10);
      printf("Colour balance ver: %d\n", n);
      break;
    case 0x00a5:  // Exif.Nikon3.ImageCount
      if (!raw_data.exif.image_count) {
        raw_data.exif.image_count = get_tag_value(tag_type);
      }
      break;
    case 0x00a7:  // Exif.Nikon3.ShutterCount
      if (!raw_data.exif.shutter_count) {
        raw_data.exif.shutter_count = get_tag_value(tag_type);
      }
      break;
    default:
      break;
  }
  file.seekg(tag_offset, std::ios::beg);
}
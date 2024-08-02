
#include "rawimagedata.h"
#include "rawimagedata_utils.h"

RawImageData :: RawImageData(const string_t& file_path) : file_path(file_path), file(file_path, std::ios::binary) {
  if (!file) {
    throw runtime_error_t("Unable to open file: " + file_path);
  }

  raw_image_file.base = 0;
  raw_image_file.bitorder = 0x4949; // default = little endian
  raw_image_file.version = 0;
  raw_image_file.first_offset = 0;
  raw_image_file.raw_ifd_count = 0;
  raw_image_file.file_size = 0;
  raw_image_file.make[0] = 0;
  raw_image_file.model[0] = 0;
  raw_image_file.software[0] = 0;
  raw_image_file.time_stamp = 0;
  raw_image_file.time_zone = 0; // default = UTC
  raw_image_file.artist[0] = 0;
  raw_image_file.copyright[0] = 0;

  raw_identify();

  if (parse_raw_image(raw_image_file.base)) {
    // apply raw frame (tiff)
  }
  
  
}

int RawImageData :: raw_identify() {

  char raw_image_header[32];
  file.seekg(0, std::ios::beg);
  file.read(raw_image_header, sizeof(raw_image_header)/sizeof(raw_image_header[0]));
  file.seekg(0, std::ios::beg);

  raw_image_file.file_size = get_file_size(file);

  raw_image_file.bitorder = read_2_bytes(file, raw_image_file.bitorder);
  if (raw_image_file.bitorder == 0x4949 || raw_image_file.bitorder == 0x4D4D) {
    // II or MM at the beginng of the file
    raw_image_file.base = 0;
    raw_image_file.version = read_2_bytes(file, raw_image_file.bitorder);
    raw_image_file.first_offset = read_4_bytes(file, raw_image_file.bitorder);
    printf("Version: %d, First Offset: %d, File Size: %d\n", raw_image_file.version, raw_image_file.first_offset, raw_image_file.file_size);
    file.seekg(raw_image_file.first_offset, std::ios::beg);
  } else {
    return 0;
  }

  return 1;
}

int RawImageData :: parse_raw_image(uint32_t ifd_base) {
  // reset ifd count
  raw_image_file.raw_ifd_count = 0; // fails since it shouldn't be 0 unless its the first call (in case of recursion)
  memset(raw_image_ifd, 0, sizeof(raw_image_ifd));
  
  uint32_t ifd_offset;

  while ((ifd_offset = parse_raw_image_ifd(ifd_base))) {
    file.seekg(ifd_offset, std::ios::beg);
  }

  return 1;
}

int RawImageData :: parse_raw_image_ifd(uint32_t ifd_base) {
  uint8_t ifd;
  uint32_t curr_ifd_offset, next_ifd_offset;
  if (raw_image_file.raw_ifd_count >= sizeof(raw_image_ifd) / sizeof(raw_image_ifd[0])) {
    fprintf(stderr, "Raw File IFD Count Exceeded\n");
    return 0;
  }
  raw_image_file.raw_ifd_count++;
  ifd = raw_image_file.raw_ifd_count;

  raw_image_ifd[ifd].n_tag_entries = read_2_bytes(file, raw_image_file.bitorder);

  for (uint16_t tag = 0; tag < raw_image_ifd[ifd].n_tag_entries; ++tag) {
    process_tag(ifd_base, ifd);
  }

  curr_ifd_offset = file.tellg();
  next_ifd_offset = read_4_bytes(file, raw_image_file.bitorder);
  file.seekg(curr_ifd_offset, std::ios::beg);

  printf("next ifd offset: %d\n", curr_ifd_offset);
  return next_ifd_offset;
}

void RawImageData :: process_tag(uint32_t ifd_base, int ifd) {
  uint32_t tag_id, tag_type, tag_count, tag_offset;
  uint64_t tag_data_offset, sub_ifd_offset;
  get_tag_header(ifd_base, &tag_id, &tag_type, &tag_count, &tag_offset);
  printf("tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
  tag_data_offset = get_tag_data_offset(ifd_base, tag_type, tag_count);
  
  file.seekg(tag_data_offset, std::ios::beg); // Jump to data offset
  switch(tag_id) {
    case 254: case 0:   // NewSubfileType
      break;
    case 255: case 1:   // SubfileType
      break;
    case 256: case 2:   // ImageWidth
      raw_image_ifd[ifd].width = get_tag_value_int(tag_type);
      printf("width: %d\n", raw_image_ifd[ifd].width);
      break;
    case 257: case 3:   // ImageLength
      raw_image_ifd[ifd].height = get_tag_value_int(tag_type);
      printf("height: %d\n", raw_image_ifd[ifd].height);
      break;
    case 258: case 4:   // BitsPerSample
      raw_image_ifd[ifd].bps = get_tag_value_int(tag_type);
      printf("bps: %d\n", raw_image_ifd[ifd].bps);
      break;
    case 259: case 5:   // Compression
      raw_image_ifd[ifd].compression = get_tag_value_int(tag_type);
      break;
    case 262: case 8:   // PhotometricInterpretation
      raw_image_ifd[ifd].pinterpret = get_tag_value_int(tag_type);
      break;
    case 271: case 17:  // Make
      file.read(raw_image_file.make, 64);
      break;
    case 272: case 18:  // Model
      file.read(raw_image_file.model, 64);
      break;
    case 273: case 19:  // StripOffsets
      raw_image_ifd[ifd].strip_offset = get_tag_value_int(tag_type) + ifd_base;
      break;
    case 274: case 20:  // Orientation
      raw_image_ifd[ifd].orientation = get_tag_value_int(tag_type);
      break;
    case 277: case 23:  // SamplesPerPixel
      raw_image_ifd[ifd].sample_pixel = get_tag_value_int(tag_type);
      break;
    case 278: case 24:  // RowsPerStrip
      raw_image_ifd[ifd].rows_per_strip = get_tag_value_int(tag_type);
      break;
    case 279: case 25:  // StripByteCounts
      raw_image_ifd[ifd].strip_byte_counts = get_tag_value_int(tag_type);
      break;
    case 282: case 28:  // XResolution
      raw_image_ifd[ifd].x_res = get_tag_value_int(tag_type);
      break;
    case 283: case 29:  // YResolution
      raw_image_ifd[ifd].y_res = get_tag_value_int(tag_type);
      break;
    case 284: case 30:  // PlanarConfiguration
      raw_image_ifd[ifd].planar_config = get_tag_value_int(tag_type);
      break;
    case 296: case 42:  // ResolutionUnit
      break;
    case 305: case 51:  // Software
      file.read(raw_image_file.software, 64);
      break;
    case 306: case 52:  // DateTime
      get_time_stamp();
      break;
    case 315: case 61:  // Artist
      file.read(raw_image_file.artist, 64);
      break;
    case 320: case 66:  // ColorMap
      break;
    case 322: case 68:  // TileWidth
      raw_image_ifd[ifd].tile_width = get_tag_value_int(tag_type); 
      break;
    case 323: case 69:  // TileLength
      raw_image_ifd[ifd].tile_length = get_tag_value_int(tag_type);
      break;
    case 324: case 70:  // TileOffsets
      break;
    case 325: case 71:  // TileByteCounts
      break;
    case 330: case 76:  // SubIFDs
      sub_ifd_offset = get_tag_value_int(tag_type);
      printf("sub ifd offset: %d\n", sub_ifd_offset);
      if (sub_ifd_offset) {
        file.seekg(sub_ifd_offset + ifd_base);
        while (sub_ifd_offset = parse_raw_image_ifd(ifd_base)) {
          file.seekg(sub_ifd_offset, std::ios::beg);
        }
      }
      break;
    case 513:           // JPEGInterchangeFormat
      raw_image_ifd[ifd].jpeg_if_offset = get_tag_value_int(tag_type);
      break;
    case 514:           // JPEGInterchangeFormatLength
      raw_image_ifd[ifd].jpeg_if_length = get_tag_value_int(tag_type);
      break;
    case 529:           // YCbCrCoefficients
      break;
    case 530:           // YCbCrSubSampling
      break;
    case 531:           // YCbCrPositioning
      break;
    case 532:           // ReferenceBlackWhite
      break;
    case 33432:         // Copyright
      file.read(raw_image_file.copyright, 64);
      printf("copyright: %s\n", raw_image_file.copyright);
      break;
    case 34665:         // Exif IFD
      break;
  }
  file.seekg(tag_offset, std::ios::beg);
}

uint64_t RawImageData :: get_tag_data_offset(uint32_t ifd_base, uint32_t tag_type, uint32_t tag_count) {
  uint32_t type_byte = 1;
  switch (tag_type) {
    case 0: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::BYTE);      break;
    case 1: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::ASCII);     break;
    case 2: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::SHORT);     break;
    case 3: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::LONG);      break;
    case 4: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::RATIONAL);  break;
    case 5: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::SBYTE);     break;
    case 6: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::UNDEFINED); break;
    case 7: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::SSHORT);    break;
    case 8: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::SLONG);     break;
    case 9: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::SRATIONAL); break;
    case 10: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::FLOAT);    break;
    case 11: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::DOUBLE);   break;
    case 12: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::IFD);      break;
    case 13: type_byte = static_cast<uint32_t>(Raw_Tag_Type_Bytes::LONG8);    break;
    default: type_byte = 1; break;
  }
  if (type_byte * tag_count > 4) {
    return read_4_bytes(file, raw_image_file.bitorder) + ifd_base;
  }
  return file.tellg();
}

void RawImageData :: get_tag_header(uint32_t ifd_base, uint32_t *tag_id, uint32_t *tag_type, uint32_t *tag_count, uint32_t *tag_offset) {
  *tag_id = read_2_bytes(file, raw_image_file.bitorder);
  *tag_type = read_2_bytes(file, raw_image_file.bitorder);
  *tag_count = read_4_bytes(file, raw_image_file.bitorder);
  *tag_offset = static_cast<int>(file.tellg()) + 4;
}

uint32_t RawImageData :: get_tag_value_int(uint32_t tag_type) {
  if (tag_type == 3) {
    // Reading LONG with 2 bytes
    return read_2_bytes(file, raw_image_file.bitorder);
  } else {
    return read_4_bytes(file, raw_image_file.bitorder);
  }
}

void RawImageData :: get_time_stamp() {
  // Proper date time format: " YYYY:MM:DD HH:MM:SS"
  char time_str[20];
  file.read(time_str, 20);

  struct tm t;
  memset(&t, 0, sizeof(t));
  if (sscanf(time_str, "%d:%d:%d %d:%d:%d",
  &t.tm_year, &t.tm_mon, &t.tm_mday, 
  &t.tm_hour, &t.tm_min, &t.tm_sec) != 6) return;

  t.tm_year -= 1900;
  t.tm_mon -= 1;
  t.tm_isdst = -1;

  if (mktime(&t) > 0) {
    raw_image_file.time_stamp = mktime(&t);
  }

}
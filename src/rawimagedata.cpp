
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

  raw_image_file.exif_offset = 0;

  exif.camera_make[0] = 0;
  exif.camera_model[0] = 0;
  exif.date_time = -1;
  exif.focal_length = -1;
  exif.exposure = -1;
  exif.f_number = -1;
  exif.iso_sensitivity = -1;
  exif.lens_model[0] = 0;

  raw_identify();

  if (parse_raw_image(raw_image_file.base)) {
    // apply raw frame (tiff)
  }
  
}

RawImageData :: ~RawImageData() {
  
}

bool RawImageData :: raw_identify() {

  char raw_image_header[32];
  file.seekg(0, std::ios::beg);
  file.read(raw_image_header, sizeof(raw_image_header)/sizeof(raw_image_header[0]));
  file.seekg(0, std::ios::beg);

  raw_image_file.file_size = get_file_size(file);

  raw_image_file.bitorder = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  if (raw_image_file.bitorder == 0x4949 || raw_image_file.bitorder == 0x4D4D) {
    // II or MM at the beginng of the file
    raw_image_file.base = 0;
    raw_image_file.version = read_2_bytes_unsigned(file, raw_image_file.bitorder);
    raw_image_file.first_offset = read_4_bytes_unsigned(file, raw_image_file.bitorder);
    printf("Version: %d, First Offset: %d, File Size: %d\n", raw_image_file.version, raw_image_file.first_offset, raw_image_file.file_size);
    
  } else {
    return false;
  }

  return true;
}

bool RawImageData :: parse_raw_image(off_t raw_image_file_base) {
  // go to the first offset
  file.seekg(raw_image_file.first_offset, std::ios::beg);
  // reset ifd count
  raw_image_file.raw_ifd_count = 0; // fails since it shouldn't be 0 unless its the first call (in case of recursion)
  memset(raw_image_ifd, 0, sizeof(raw_image_ifd));
  
  off_t ifd_offset;

  while ((ifd_offset = parse_raw_image_ifd(raw_image_file_base))) {
    file.seekg(ifd_offset, std::ios::beg);
  }

  return true;
}

u_int64_t RawImageData :: parse_raw_image_ifd(off_t raw_image_file_base) {
  u_int8_t ifd;
  off_t curr_ifd_offset, next_ifd_offset;
  if (raw_image_file.raw_ifd_count >= sizeof(raw_image_ifd) / sizeof(raw_image_ifd[0])) {
    fprintf(stderr, "Raw File IFD Count Exceeded\n");
    return 0;
  }
  raw_image_file.raw_ifd_count++;
  ifd = raw_image_file.raw_ifd_count;

  raw_image_ifd[ifd].n_tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);

  for (u_int16_t tag = 0; tag < raw_image_ifd[ifd].n_tag_entries; ++tag) {
    parse_raw_image_tag(raw_image_file_base, ifd);
  }

  curr_ifd_offset = file.tellg();
  next_ifd_offset = read_4_bytes_unsigned(file, raw_image_file.bitorder) + raw_image_file_base;
  file.seekg(curr_ifd_offset, std::ios::beg);

  printf("next ifd offset: %d\n", curr_ifd_offset);
  return next_ifd_offset;
}

void RawImageData :: parse_raw_image_tag(off_t raw_image_file_base, u_int64_t ifd) {
  u_int32_t tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset, sub_ifd_offset;
  get_tag_header(raw_image_file_base, &tag_id, &tag_type, &tag_count, &tag_offset);
  printf("tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
  tag_data_offset = get_tag_data_offset(raw_image_file_base, tag_type, tag_count);
  
  file.seekg(tag_data_offset, std::ios::beg); // Jump to data offset
  switch(tag_id) {
    case 254: case 0:   // NewSubfileType
      break;
    case 255: case 1:   // SubfileType
      break;
    case 256: case 2:   // ImageWidth
      raw_image_ifd[ifd].width = get_tag_value(tag_type);
      printf("width: %d\n", raw_image_ifd[ifd].width);
      break;
    case 257: case 3:   // ImageLength
      raw_image_ifd[ifd].height = get_tag_value(tag_type);
      printf("height: %d\n", raw_image_ifd[ifd].height);
      break;
    case 258: case 4:   // BitsPerSample
      raw_image_ifd[ifd].bps = get_tag_value(tag_type);
      printf("bps: %d\n", raw_image_ifd[ifd].bps);
      break;
    case 259: case 5:   // Compression
      raw_image_ifd[ifd].compression = get_tag_value(tag_type);
      printf("compression: %d\n", raw_image_ifd[ifd].compression);
      break;
    case 262: case 8:   // PhotometricInterpretation
      raw_image_ifd[ifd].pinterpret = get_tag_value(tag_type);
      break;
    case 271: case 17:  // Make
      file.read(raw_image_file.make, 64);
      break;
    case 272: case 18:  // Model
      file.read(raw_image_file.model, 64);
      break;
    case 273: case 19:  // StripOffsets
      raw_image_ifd[ifd].strip_offset = get_tag_value(tag_type) + raw_image_file_base;
      printf("strip offset: %d\n", raw_image_ifd[ifd].strip_offset);
      parse_strip_data(raw_image_file_base, ifd);
      break;
    case 274: case 20:  // Orientation
      raw_image_ifd[ifd].orientation = get_tag_value(tag_type);
      break;
    case 277: case 23:  // SamplesPerPixel
      raw_image_ifd[ifd].sample_pixel = get_tag_value(tag_type);
      printf("samples per pixel: %d\n", raw_image_ifd[ifd].sample_pixel);
      break;
    case 278: case 24:  // RowsPerStrip
      raw_image_ifd[ifd].rows_per_strip = get_tag_value(tag_type);
      break;
    case 279: case 25:  // StripByteCounts
      raw_image_ifd[ifd].strip_byte_counts = get_tag_value(tag_type);
      break;
    case 282: case 28:  // XResolution
      raw_image_ifd[ifd].x_res = get_tag_value(tag_type);
      break;
    case 283: case 29:  // YResolution
      raw_image_ifd[ifd].y_res = get_tag_value(tag_type);
      break;
    case 284: case 30:  // PlanarConfiguration
      raw_image_ifd[ifd].planar_config = get_tag_value(tag_type);
      break;
    case 296: case 42:  // ResolutionUnit
      break;
    case 305: case 51:  // Software
      file.read(raw_image_file.software, 64);
      break;
    case 306: case 52:  // DateTime
      parse_time_stamp();
      break;
    case 315: case 61:  // Artist
      file.read(raw_image_file.artist, 64);
      break;
    case 320: case 66:  // ColorMap
      break;
    case 322: case 68:  // TileWidth
      raw_image_ifd[ifd].tile_width = get_tag_value(tag_type); 
      break;
    case 323: case 69:  // TileLength
      raw_image_ifd[ifd].tile_length = get_tag_value(tag_type);
      break;
    case 324: case 70:  // TileOffsets
      raw_image_ifd[ifd].tile_offset = get_tag_value(tag_type) + raw_image_file_base;
      break;
    case 325: case 71:  // TileByteCounts
      break;
    case 330: case 76:  // SubIFDs
      sub_ifd_offset = get_tag_value(tag_type) + raw_image_file_base;
      printf("sub ifd offset: %d\n", sub_ifd_offset);
      if (sub_ifd_offset) {
        file.seekg(sub_ifd_offset, std::ios::beg);
        while (sub_ifd_offset = parse_raw_image_ifd(raw_image_file_base)) {
          file.seekg(sub_ifd_offset, std::ios::beg);
        }
      }
      break;
    case 513:           // JPEGInterchangeFormat
      raw_image_ifd[ifd].jpeg_if_offset = get_tag_value(tag_type) + raw_image_file_base;
      break;
    case 514:           // JPEGInterchangeFormatLength
      raw_image_ifd[ifd].jpeg_if_length = get_tag_value(tag_type);
      break;
    case 529:           // YCbCrCoefficients
      break;
    case 530:           // YCbCrSubSampling
      break;
    case 531:           // YCbCrPositioning
      break;
    case 532:           // ReferenceBlackWhite
      break;
    case 33421:         // CFARepeatPatternDim
      break;
    case 33422:         // CFAPattern
      break;
    case 33432:         // Copyright
      file.read(raw_image_file.copyright, 64);
      break;
    case 33434:         // ExposureTime
      exif.exposure = get_tag_value(tag_type);
      break;
    case 33437:         // FNumber
      exif.f_number = get_tag_value(tag_type);
      break;
    case 34665:         // Exif IFD
      raw_image_file.exif_offset = get_tag_value(tag_type) + raw_image_file_base;
      parse_exif_data(raw_image_file_base);
      break;
    case 34675:         // InterColorProfile
      exif.icc_profile_offset = file.tellg();
      exif.icc_profile_count = tag_count;
      break;
    case 34853:         // GPSInfo / GPS IFD
      exif.gps_offset = get_tag_value(tag_type) + raw_image_file_base;
      parse_gps_data(raw_image_file_base);
      break;
    
    
    default:
      break;
  }
  file.seekg(tag_offset, std::ios::beg);
}

bool RawImageData :: parse_strip_data(off_t raw_image_file_base, u_int64_t ifd) {
  jpeg_header_t jpeg_header;
  
  if (!raw_image_ifd[ifd].bps && raw_image_ifd[ifd].strip_offset > 0) {
    file.seekg(raw_image_ifd[ifd].strip_offset, std::ios::beg);
    if (get_jpeg_header(&jpeg_header, true)) {

      parse_raw_image(raw_image_ifd[ifd].strip_offset + 12);
    }
    return true;
  }
  return false;
}

bool RawImageData :: parse_exif_data(off_t raw_image_file_base) {
  u_int32_t tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;
  
  file.seekg(raw_image_file.exif_offset, std::ios::beg);
  tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  for (int i = 0; i < tag_entries; ++i) {
    get_tag_header(raw_image_file_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    tag_data_offset = get_tag_data_offset(raw_image_file_base, tag_type, tag_count);
    file.seekg(tag_data_offset, std::ios::beg);

    switch (tag_id) {
      case 0x829a:  // ExposureTime
        exif.exposure = get_tag_value(tag_type);
        break;
      case 0x829d:  // FNumber
        exif.f_number = get_tag_value(tag_type);
        break;
      case 0x8822:  // ExposureProgram
        break;
      case 0x8827:  // ISO
        exif.iso_sensitivity = get_tag_value(tag_type);
        break;
      case 0x8833:  // ISOSpeed
      case 0x8834:  // ISOSpeedLatitudeyyy
        parse_time_stamp();
        break;
      case 0x9201:  // ShutterSpeedValue
        double exposure;
        if ((exposure = -get_tag_value(tag_type)) < 128) {
          exif.exposure = pow(2, exposure);
        }
        break;
      case 0x9202:  // ApertureValue
        exif.f_number = pow(2, get_tag_value(tag_type) / 2);
        break;
      case 0x920a:  // FocalLength
        exif.focal_length = get_tag_value(tag_type);
        break;
      case 0x927c:  // MakerNote
        break;
      case 0x9286:  // UserComment
        break;
      case 0xa302:  // CFAPattern
        break;

      default:
        break;
    }

    file.seekg(tag_offset, std::ios::beg); 
  }
  return true;
}

bool RawImageData :: parse_gps_data(off_t raw_image_file_base) {
  u_int32_t tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;
  int data;
  
  file.seekg(exif.gps_offset, std::ios::beg);
  tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  for (int i = 0; i < tag_entries; ++i) {
    get_tag_header(raw_image_file_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    tag_data_offset = get_tag_data_offset(raw_image_file_base, tag_type, tag_count);
    printf("gps tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
    file.seekg(tag_data_offset, std::ios::beg);

    switch (tag_id) {
      case 0:
        for (int i = 0; i < 4; i++) {
          data = get_tag_value(tag_type);
          printf("gps data: %d\n", data);
        }
        break;
    
      default:
        break;
    }

    file.seekg(tag_offset, std::ios::beg);
  }
  return true;
}

bool RawImageData :: parse_time_stamp() {
  // Proper date time format: " YYYY:MM:DD HH:MM:SS"
  char time_str[20];
  file.read(time_str, 20);

  struct tm t;
  memset(&t, 0, sizeof(t));
  if (sscanf(time_str, "%d:%d:%d %d:%d:%d",
  &t.tm_year, &t.tm_mon, &t.tm_mday, 
  &t.tm_hour, &t.tm_min, &t.tm_sec) != 6) return false;

  t.tm_year -= 1900;
  t.tm_mon -= 1;
  t.tm_isdst = -1;

  if (mktime(&t) > 0) {
    raw_image_file.time_stamp = mktime(&t);
  }

  return true;
}

off_t RawImageData :: get_tag_data_offset(off_t raw_image_file_base, u_int32_t tag_type, u_int32_t tag_count) {
  u_int32_t type_byte = 1;
  switch (tag_type) {
    case 1:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::BYTE);     break;
    case 2:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::ASCII);    break;
    case 3:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::SHORT);    break;
    case 4:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::LONG);     break;
    case 5:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::RATIONAL); break;
    case 6:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::SBYTE);    break;
    case 7:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::UNDEFINED);break;
    case 8:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::SSHORT);   break;
    case 9:   type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::SLONG);    break;
    case 10:  type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::SRATIONAL);break;
    case 11:  type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::FLOAT);    break;
    case 12:  type_byte = static_cast<u_int32_t>(Raw_Tag_Type_Bytes::DOUBLE);   break;
    
    default: type_byte = 1; break;
  }
  if (type_byte * tag_count > 4) {
    return read_4_bytes_unsigned(file, raw_image_file.bitorder) + raw_image_file_base;
  }
  return file.tellg();
}

void RawImageData :: get_tag_header(off_t raw_image_file_base, u_int32_t *tag_id, u_int32_t *tag_type, u_int32_t *tag_count, off_t *tag_offset) {
  *tag_id = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  *tag_type = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  *tag_count = read_4_bytes_unsigned(file, raw_image_file.bitorder);
  *tag_offset = static_cast<int>(file.tellg()) + 4;
}

double RawImageData :: get_tag_value(u_int32_t tag_type) {
  double numerator, denominator;
  char byte_array[8];
  int i, reverse_order;
  switch (tag_type) {
    case 1: // BYTE
      return read_1_byte_unsigned(file, raw_image_file.bitorder);
    case 2: // ASCII
      return read_1_byte_unsigned(file, raw_image_file.bitorder);
    case 3: // SHORT
      return read_2_bytes_unsigned(file, raw_image_file.bitorder);
    case 4: // LONG
      return read_4_bytes_unsigned(file, raw_image_file.bitorder);
    case 5: // RATIONAL
      numerator = read_4_bytes_unsigned(file, raw_image_file.bitorder);
      denominator = read_4_bytes_unsigned(file, raw_image_file.bitorder);
      return numerator / denominator;
    case 6: // SBYTE
      return read_1_byte_signed(file, raw_image_file.bitorder);
    case 7: // UNDEFINED
      return read_1_byte_unsigned(file, raw_image_file.bitorder);
    case 8: // SSHORT
      return read_2_byte_signed(file, raw_image_file.bitorder);
    case 9: // SLONG
      return read_4_byte_signed(file, raw_image_file.bitorder);
    case 10:// SRATIONAL
      numerator = read_4_byte_signed(file, raw_image_file.bitorder);
      denominator = read_4_byte_signed(file, raw_image_file.bitorder);
      return numerator / denominator;
    case 11://FLOAT
      return *reinterpret_cast<float*>(read_4_bytes_unsigned(file, raw_image_file.bitorder));
    case 12://DOUBLE
      reverse_order = 7 * ((raw_image_file.bitorder == 0x4949) == (ntohs(0x1234) == 0x1234));
      for (i = 0; i < 8; i++) {
        int index = i ^ reverse_order;
        byte_array[index] = read_1_byte_unsigned(file, raw_image_file.bitorder);
      }
      double value;
      memcpy(&value, byte_array, sizeof(value));
      return value;

    default:
      return read_1_byte_signed(file, raw_image_file.bitorder);
  }
}

/**
 * https://yasoob.me/posts/understanding-and-writing-jpeg-decoder-in-python/
 */
bool RawImageData :: get_jpeg_header(jpeg_header_t* jpeg_header, bool info_only) {
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

  int len = 0;
  while (file.read(reinterpret_cast<char*>(&buffer), 2)) {
    if (buffer[0] != 0xff) {
      continue;
    }
    switch (buffer[1]) {
      case 0xC0:  // SOF0
      case 0xC1:  // SOF1
        file.read(reinterpret_cast<char*>(&buffer), 2);
        file.read(reinterpret_cast<char*>(&buffer), 1);
        jpeg_header->bits = buffer[0];
        file.read(reinterpret_cast<char*>(&buffer), 2);
        jpeg_header->height = (buffer[0] << 8 | buffer[1]);
        file.read(reinterpret_cast<char*>(&buffer), 2);
        jpeg_header->width = (buffer[0] << 8 | buffer[1]);
        file.read(reinterpret_cast<char*>(&buffer), 1);
        jpeg_header->clrs = buffer[0];
        break;
      case 0xC4:  // Huffman Table
        file.read(reinterpret_cast<char*>(&buffer), 2);
        len = (buffer[0] << 8 | buffer[1]);
        len -= 2;
        break;
    
      default:
        // Skip over other markers
        file.read(reinterpret_cast<char*>(buffer), 2);
        int length = (buffer[0] << 8) | buffer[1];
        file.seekg(length - 2, std::ios_base::cur);
        break;
    }
  }

  if (jpeg_header->bits > 16 || jpeg_header == 0) {
    return false;
  }
  if (jpeg_header->clrs > 6 || jpeg_header->clrs == 0) {
    return false;
  }
  if (jpeg_header->height == 0 || !jpeg_header->width == 0) {
    return false;
  }
  if (jpeg_header->huff[0] == 0) {
    return false;
  }

  return true;
}


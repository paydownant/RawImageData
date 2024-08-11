
#include "rawimagedata.h"

RawImageData :: RawImageData(const std::string& file_path) : file_path(file_path), file(file_path, std::ios::binary) {
  if (!file) {
    throw std::runtime_error("Unable to open file: " + file_path);
  }

  raw_identify();
  if (parse_raw(raw_image_file.base)) {
    // print_data();
    // apply raw frame (tiff)
  }
}

RawImageData :: ~RawImageData() {
  
}

bool RawImageData :: raw_identify() {
  char raw_image_header[32];
  raw_image_file.bitorder = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  file.seekg(0, std::ios::beg);
  file.read(raw_image_header, sizeof(raw_image_header)/sizeof(raw_image_header[0]));

  file.seekg(0, std::ios::end);
  raw_image_file.file_size = file.tellg();

  if (raw_image_file.bitorder == 0x4949 || raw_image_file.bitorder == 0x4D4D) {
    // II or MM at the beginng of the file
    raw_image_file.base = 0;
  } else {
    return false;
  }

  return true;
}

bool RawImageData :: parse_raw(off_t raw_image_file_base) {
  // reset ifd count
  raw_image_file.raw_ifd_count = 0; // fails since it shouldn't be 0 unless its the first call (in case of recursion)
  memset(raw_image_file.raw_image_ifd, 0, sizeof(raw_image_file.raw_image_ifd));

  file.seekg(0, std::ios::beg);
  parse_raw_image(raw_image_file_base);

  return true;
}

bool RawImageData :: parse_raw_image(off_t raw_image_file_base) {
  // go to the base
  file.seekg(raw_image_file_base, std::ios::beg);
  
  u_int curr_bitorder, version;
  off_t ifd_offset;
  
  curr_bitorder = raw_image_file.bitorder;
  raw_image_file.bitorder = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  
  if (raw_image_file.bitorder != 0x4949 && raw_image_file.bitorder != 0x4d4d) {
    raw_image_file.bitorder = curr_bitorder;
    return false;
  }

  version = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  // ifd_offset = read_4_bytes_unsigned(file, raw_image_file.bitorder);

  while ((ifd_offset = read_4_bytes_unsigned(file, raw_image_file.bitorder))) {
    file.seekg(ifd_offset + raw_image_file_base, std::ios::beg);
    if (!parse_raw_image_ifd(raw_image_file_base)) {
      break;
    }
  }

  return true;
}

bool RawImageData :: parse_raw_image_ifd(off_t raw_image_file_base) {
  u_int ifd;
  if (raw_image_file.raw_ifd_count >= sizeof(raw_image_file.raw_image_ifd) / sizeof(raw_image_file.raw_image_ifd[0])) {
    fprintf(stderr, "Raw File IFD Count Exceeded\n");
    return false;
  }
  raw_image_file.raw_ifd_count++;
  ifd = raw_image_file.raw_ifd_count;

  if (raw_image_file.raw_image_ifd[ifd].set) {
    fprintf(stderr, "ERROR: Raw Image IFD Already SET\n");
    return false;
  }

  u_int n_tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  if (n_tag_entries != 0) {
    raw_image_file.raw_image_ifd[ifd].set = true;
    raw_image_file.raw_image_ifd[ifd].n_tag_entries = n_tag_entries;
  }

  for (u_int tag = 0; tag < raw_image_file.raw_image_ifd[ifd].n_tag_entries; ++tag) {
    parse_raw_image_tag(raw_image_file_base, ifd);
  }

  return true;
}

void RawImageData :: parse_raw_image_tag(off_t raw_image_file_base, u_int ifd) {
  u_int tag_id, tag_type, tag_count;
  off_t offset, tag_data_offset, tag_offset, sub_ifd_offset;
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
      raw_image_file.raw_image_ifd[ifd].width = get_tag_value(tag_type);
      break;
    case 257: case 3:   // ImageLength
      raw_image_file.raw_image_ifd[ifd].height = get_tag_value(tag_type);
      break;
    case 258: case 4:   // BitsPerSample
      raw_image_file.raw_image_ifd[ifd].bps = get_tag_value(tag_type);
      break;
    case 259: case 5:   // Compression
      raw_image_file.raw_image_ifd[ifd].compression = get_tag_value(tag_type);
      break;
    case 262: case 8:   // PhotometricInterpretation
      raw_image_file.raw_image_ifd[ifd].pinterpret = get_tag_value(tag_type);
      break;
    case 271: case 17:  // Make
      file.read(raw_image_file.make, 64);
      break;
    case 272: case 18:  // Model
      file.read(raw_image_file.model, 64);
      break;
    case 273: case 19:  // StripOffsets
      raw_image_file.raw_image_ifd[ifd].strip_offset = get_tag_value(tag_type) + raw_image_file_base;
      parse_strip_data(raw_image_file_base, ifd);
      break;
    case 274: case 20:  // Orientation
      raw_image_file.raw_image_ifd[ifd].orientation = get_tag_value(tag_type);
      break;
    case 277: case 23:  // SamplesPerPixel
      raw_image_file.raw_image_ifd[ifd].sample_pixel = get_tag_value(tag_type);
      break;
    case 278: case 24:  // RowsPerStrip
      raw_image_file.raw_image_ifd[ifd].rows_per_strip = get_tag_value(tag_type);
      break;
    case 279: case 25:  // StripByteCounts
      raw_image_file.raw_image_ifd[ifd].strip_byte_counts = get_tag_value(tag_type);
      break;
    case 282: case 28:  // XResolution
      raw_image_file.raw_image_ifd[ifd].x_res = get_tag_value(tag_type);
      break;
    case 283: case 29:  // YResolution
      raw_image_file.raw_image_ifd[ifd].y_res = get_tag_value(tag_type);
      break;
    case 284: case 30:  // PlanarConfiguration
      raw_image_file.raw_image_ifd[ifd].planar_config = get_tag_value(tag_type);
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
      raw_image_file.raw_image_ifd[ifd].tile_width = get_tag_value(tag_type); 
      break;
    case 323: case 69:  // TileLength
      raw_image_file.raw_image_ifd[ifd].tile_length = get_tag_value(tag_type);
      break;
    case 324: case 70:  // TileOffsets
      raw_image_file.raw_image_ifd[ifd].tile_offset = get_tag_value(tag_type) + raw_image_file_base;
      break;
    case 325: case 71:  // TileByteCounts
      break;
    case 330: case 76:  // SubIFDs
      while (tag_count--) {
        offset = file.tellg();
        sub_ifd_offset = read_4_bytes_unsigned(file, raw_image_file.bitorder) + raw_image_file_base;
        file.seekg(sub_ifd_offset, std::ios::beg);
        if (!parse_raw_image_ifd(raw_image_file_base)) {
          break;
        }
        file.seekg(offset + 4, std::ios::beg);
      }
      break;
    case 513:           // JPEGInterchangeFormat
      raw_image_file.raw_image_ifd[ifd].strip_offset = get_tag_value(tag_type) + raw_image_file_base;
      parse_strip_data(raw_image_file_base, ifd);
      break;
    case 514:           // JPEGInterchangeFormatLength
      raw_image_file.raw_image_ifd[ifd].jpeg_if_length = get_tag_value(tag_type);
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
      raw_image_file.exif.exposure = get_tag_value(tag_type);
      break;
    case 33437:         // FNumber
      raw_image_file.exif.f_number = get_tag_value(tag_type);
      break;
    case 34665:         // Exif IFD
      raw_image_file.exif_offset = get_tag_value(tag_type) + raw_image_file_base;
      parse_exif_data(raw_image_file_base);
      break;
    case 34675:         // InterColorProfile
      raw_image_file.exif.icc_profile_offset = file.tellg();
      raw_image_file.exif.icc_profile_count = tag_count;
      break;
    case 34853:         // GPSInfo / GPS IFD
      raw_image_file.exif.gps_offset = get_tag_value(tag_type) + raw_image_file_base;
      parse_gps_data(raw_image_file_base);
      break;
    
    default:
      break;
  }
  file.seekg(tag_offset, std::ios::beg);
}

bool RawImageData :: parse_strip_data(off_t raw_image_file_base, u_int ifd) {
  jpeg_info_t jpeg_info;
  if (raw_image_file.raw_image_ifd[ifd].bps || raw_image_file.raw_image_ifd[ifd].strip_offset == 0) {
    return false;
  }
  file.seekg(raw_image_file.raw_image_ifd[ifd].strip_offset, std::ios::beg);
  if (parse_jpeg_info(file, &jpeg_info, true)) {
    // print_jpeg_info(&jpeg_info);
    raw_image_file.raw_image_ifd[ifd].compression = 6;
    raw_image_file.raw_image_ifd[ifd].width = jpeg_info.width;
    raw_image_file.raw_image_ifd[ifd].height = jpeg_info.height;
    raw_image_file.raw_image_ifd[ifd].bps = jpeg_info.precision;
    raw_image_file.raw_image_ifd[ifd].sample_pixel = jpeg_info.components;

    parse_raw_image(raw_image_file.raw_image_ifd[ifd].strip_offset + 12);
  }
  
  return true;
}

bool RawImageData :: parse_exif_data(off_t raw_image_file_base) {
  u_int n_tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;
  
  file.seekg(raw_image_file.exif_offset, std::ios::beg);
  n_tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  for (int i = 0; i < n_tag_entries; ++i) {
    get_tag_header(raw_image_file_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    tag_data_offset = get_tag_data_offset(raw_image_file_base, tag_type, tag_count);
    file.seekg(tag_data_offset, std::ios::beg);

    switch (tag_id) {
      case 0x829a:  // ExposureTime
        raw_image_file.exif.exposure = get_tag_value(tag_type);
        break;
      case 0x829d:  // FNumber
        raw_image_file.exif.f_number = get_tag_value(tag_type);
        break;
      case 0x8822:  // ExposureProgram
        break;
      case 0x8827:  // ISO
        raw_image_file.exif.iso_sensitivity = get_tag_value(tag_type);
        break;
      case 0x8833:  // ISOSpeed
      case 0x8834:  // ISOSpeedLatitudeyyy
        parse_time_stamp();
        break;
      case 0x9201:  // ShutterSpeedValue
        double exposure;
        if ((exposure = -get_tag_value(tag_type)) < 128) {
          raw_image_file.exif.exposure = pow(2, exposure);
        }
        break;
      case 0x9202:  // ApertureValue
        raw_image_file.exif.f_number = pow(2, get_tag_value(tag_type) / 2);
        break;
      case 0x920a:  // FocalLength
        raw_image_file.exif.focal_length = get_tag_value(tag_type);
        break;
      case 0x927c:  // MakerNote
        parse_makernote(raw_image_file_base, 0);
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

bool RawImageData :: parse_makernote(off_t raw_image_file_base, int uptag) {
  char maker_magic[10];
  off_t base, offset;
  file.read(maker_magic, 10);
  printf("Parse Maker Note: %s, Offset: %d\n", maker_magic, file.tellg());

  if (!strncasecmp(maker_magic, "Nikon", 6)) {
    printf("Nikon Found\n");
    base = file.tellg();
    raw_image_file.bitorder = read_2_bytes_unsigned(file, raw_image_file.bitorder);
    read_2_bytes_unsigned(file, raw_image_file.bitorder);
    offset = read_4_bytes_unsigned(file, raw_image_file.bitorder);
    if (offset != 8) {
      return false;
    }

  } else if (!strncasecmp(maker_magic, "Olympus", 8)) {
    printf("Olympus Found\n");

  }


  if (!strncasecmp(maker_magic, "Nikon", 6)) {
    parse_markernote_tags_nikon(raw_image_file_base);
  }

  return true;
}

void RawImageData :: parse_markernote_tags_nikon(off_t raw_image_file_base) {
  u_int n_tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;

  n_tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  for (int i = 0; i < n_tag_entries; ++i) {
    get_tag_header(raw_image_file_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    printf("Makernote tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
  
  
    file.seekg(tag_offset, std::ios::beg);
  }
}

bool RawImageData :: parse_gps_data(off_t raw_image_file_base) {
  u_int n_tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;
  int data;
  file.seekg(raw_image_file.exif.gps_offset, std::ios::beg);
  n_tag_entries = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  for (int i = 0; i < n_tag_entries; ++i) {
    get_tag_header(raw_image_file_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    tag_data_offset = get_tag_data_offset(raw_image_file_base, tag_type, tag_count);
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

off_t RawImageData :: get_tag_data_offset(off_t raw_image_file_base, u_int tag_type, u_int tag_count) {
  u_int type_byte = 1;
  switch (tag_type) {
    case 1:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::BYTE);     break;
    case 2:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::ASCII);    break;
    case 3:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::SHORT);    break;
    case 4:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::LONG);     break;
    case 5:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::RATIONAL); break;
    case 6:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::SBYTE);    break;
    case 7:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::UNDEFINED);break;
    case 8:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::SSHORT);   break;
    case 9:   type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::SLONG);    break;
    case 10:  type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::SRATIONAL);break;
    case 11:  type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::FLOAT);    break;
    case 12:  type_byte = static_cast<u_int>(Raw_Tag_Type_Bytes::DOUBLE);   break;
    
    default: type_byte = 1; break;
  }
  if (type_byte * tag_count > 4) {
    return read_4_bytes_unsigned(file, raw_image_file.bitorder) + raw_image_file_base;
  }
  return file.tellg();
}

void RawImageData :: get_tag_header(off_t raw_image_file_base, u_int *tag_id, u_int *tag_type, u_int *tag_count, off_t *tag_offset) {
  *tag_id = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  *tag_type = read_2_bytes_unsigned(file, raw_image_file.bitorder);
  *tag_count = read_4_bytes_unsigned(file, raw_image_file.bitorder);
  *tag_offset = static_cast<int>(file.tellg()) + 4;
}

double RawImageData :: get_tag_value(u_int tag_type) {
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


void RawImageData :: print_data() {
  printf("\n================RAW FILE DATA===============\n");
  printf("Base offset: %d\n", raw_image_file.base);
  printf("Bit ordering: 0x%x\n", raw_image_file.bitorder);
  printf("Version: %d\n", raw_image_file.version);
  printf("File size: %.2lfMB\n", (float)raw_image_file.file_size / 1000000);
  printf("Make: %s\n", raw_image_file.make);
  printf("Model: %s\n", raw_image_file.model);
  printf("Software: %s\n", raw_image_file.software);
  printf("Artist: %s\n", raw_image_file.artist);
  printf("Copyright: %s\n", raw_image_file.copyright);

  printf("\n================EXIF DATA===============\n");
  printf("Camera Make: %s\n", raw_image_file.exif.camera_make);
  printf("Camera Model: %s\n", raw_image_file.exif.camera_model);
  printf("Date time: %d\n", raw_image_file.exif.date_time);
  printf("Exposure: %lf\n", raw_image_file.exif.exposure);
  printf("F Number: %lf\n", raw_image_file.exif.f_number);
  printf("ISO Sensitivity: %d\n", raw_image_file.exif.iso_sensitivity);
  printf("Lens Model: %s\n", raw_image_file.exif.lens_model);

  printf("\n================RAW TIFF IFDs===============\n");
  for (u_int i = 0; i < 8; ++i) {
    if (raw_image_file.raw_image_ifd[i].set) {
      printf("IFD: %d\n", i + 1);
      printf("N Tag Entries: %d\n", raw_image_file.raw_image_ifd[i].n_tag_entries);
      printf("Width: %d\n", raw_image_file.raw_image_ifd[i].width);
      printf("Height: %d\n", raw_image_file.raw_image_ifd[i].height);
      printf("BPS: %d\n", raw_image_file.raw_image_ifd[i].bps);
      printf("Compression: %d\n", raw_image_file.raw_image_ifd[i].compression);
      printf("P Interpret: %d\n", raw_image_file.raw_image_ifd[i].pinterpret);
      printf("Orientation: %d\n", raw_image_file.raw_image_ifd[i].orientation);
      printf("Strip Offset: %d\n", raw_image_file.raw_image_ifd[i].strip_offset);
      printf("Tile Offset: %d\n", raw_image_file.raw_image_ifd[i].tile_offset);
      printf("\n");
    }
  }
}

#include "rawimagedata.h"

RawImageData :: RawImageData(const std::string& file_path) : file_path(file_path), file(file_path, std::ios::binary) {
  if (!file) {
    throw std::runtime_error("Unable to open file: " + file_path);
  }
}

RawImageData :: ~RawImageData() {}

bool RawImageData :: load_raw() {
  raw_identify();
  if (parse_raw(raw_data.base)) {
    // apply raw frame (tiff)
    apply_raw_data();
    print_data(true, true, false);
  }

  return true;
}

bool RawImageData :: raw_identify() {
  char raw_image_header[32];
  raw_data.bitorder = read_2_bytes_unsigned(file, raw_data.bitorder);
  file.seekg(0, std::ios::beg);
  file.read(raw_image_header, sizeof(raw_image_header)/sizeof(raw_image_header[0]));

  file.seekg(0, std::ios::end);
  raw_data.file_size = file.tellg();

  if (raw_data.bitorder == 0x4949 || raw_data.bitorder == 0x4D4D) {
    // II or MM at the beginng of the file
    raw_data.base = 0;
  } else {
    return false;
  }

  return true;
}

bool RawImageData :: apply_raw_data() {

  u_int max_size = 0, cur_size = 0;

  for (u_int ifd = 0; ifd < raw_data.ifd_count; ++ifd) {
    printf("Apply IFD: %d |",ifd);
    if (!raw_data.ifds[ifd].set) {
      /* Skip unset ifd */
      printf(" SKIP\n");
      continue;
    }
    
    /* Get pixel */
    cur_size = raw_data.ifds[ifd].frame.width * raw_data.ifds[ifd].frame.height * raw_data.ifds[ifd].frame.bps;
    /* APPLY FRAME */
    if (cur_size > max_size && (raw_data.ifds[ifd].frame.bps != 6 || raw_data.ifds[ifd].frame.sample_pixel != 3)) {
      raw_data.main_frame = raw_data.ifds[ifd].frame;
      raw_data.data_offset = raw_data.ifds[ifd].data_offset;
      max_size = cur_size;
      printf(" Max IFD Set ");
    }

    /* APPLY EXIF */
    if (raw_data.ifds[ifd].exif.camera_make[0] != 0) {
      strcpy(raw_data.main_exif.camera_make, raw_data.ifds[ifd].exif.camera_make);
    }
    if (raw_data.ifds[ifd].exif.camera_model[0] != 0) {
      strcpy(raw_data.main_exif.camera_model, raw_data.ifds[ifd].exif.camera_model);
    }
    if (raw_data.ifds[ifd].exif.software[0] != 0) {
      strcpy(raw_data.main_exif.software, raw_data.ifds[ifd].exif.software);
    }
    if (raw_data.ifds[ifd].exif.focal_length != 0) {
      raw_data.main_exif.focal_length = raw_data.ifds[ifd].exif.focal_length;
    }
    if (raw_data.ifds[ifd].exif.exposure != 0) {
      raw_data.main_exif.exposure = raw_data.ifds[ifd].exif.exposure;
    }
    if (raw_data.ifds[ifd].exif.f_number != 0) {
      raw_data.main_exif.f_number = raw_data.ifds[ifd].exif.f_number;
    }
    if (raw_data.ifds[ifd].exif.iso_sensitivity != 0) {
      raw_data.main_exif.iso_sensitivity = raw_data.ifds[ifd].exif.iso_sensitivity;
    }
    if (raw_data.ifds[ifd].exif.lens_info.set) {
      raw_data.main_exif.lens_info = raw_data.ifds[ifd].exif.lens_info;
    }
    if (raw_data.ifds[ifd].exif.image_count != 0) {
      raw_data.main_exif.image_count = raw_data.ifds[ifd].exif.image_count;
    }
    if (raw_data.ifds[ifd].exif.shutter_count != 0) {
      raw_data.main_exif.shutter_count = raw_data.ifds[ifd].exif.shutter_count;
    }
    if (raw_data.ifds[ifd].exif.artist[0] != 0) {
      strcpy(raw_data.main_exif.artist, raw_data.ifds[ifd].exif.artist);
    }
    if (raw_data.ifds[ifd].exif.copyright[0] != 0) {
      strcpy(raw_data.main_exif.copyright, raw_data.ifds[ifd].exif.copyright);
    }
    if (raw_data.ifds[ifd].exif.date_time != 0) {
      raw_data.main_exif.date_time = raw_data.ifds[ifd].exif.date_time;
    }
    if (raw_data.ifds[ifd].exif.date_time_str[0] != 0) {
      strcpy(raw_data.main_exif.date_time_str, raw_data.ifds[ifd].exif.date_time_str);
    }
    if (raw_data.ifds[ifd].exif.icc_profile_offset != 0) {
      raw_data.main_exif.icc_profile_offset = raw_data.ifds[ifd].exif.icc_profile_offset;
    }
    if (raw_data.ifds[ifd].exif.icc_profile_count != 0) {
      raw_data.main_exif.icc_profile_count = raw_data.ifds[ifd].exif.icc_profile_count;
    }
    if (raw_data.ifds[ifd].exif.gps_latitude_reference != 0) {
      raw_data.main_exif.gps_latitude_reference = raw_data.ifds[ifd].exif.gps_latitude_reference;
    }
    if (raw_data.ifds[ifd].exif.gps_latitude != 0) {
      raw_data.main_exif.gps_latitude = raw_data.ifds[ifd].exif.gps_latitude;
    }
    if (raw_data.ifds[ifd].exif.gps_longitude_reference != 0) {
      raw_data.main_exif.gps_longitude_reference = raw_data.ifds[ifd].exif.gps_longitude_reference;
    }
    if (raw_data.ifds[ifd].exif.gps_longitude != 0) {
      raw_data.main_exif.gps_longitude = raw_data.ifds[ifd].exif.gps_longitude;
    }
    /* END OF EXIF */

    /* APPLY UTIL */
    if (raw_data.ifds[ifd].util.cfa != 0) {
      raw_data.main_util.cfa = raw_data.ifds[ifd].util.cfa;
    }
    if (raw_data.ifds[ifd].util.white_balance_multi_cam.set) {
      raw_data.main_util.white_balance_multi_cam = raw_data.ifds[ifd].util.white_balance_multi_cam;
    }
    if (raw_data.ifds[ifd].util.cblack.set) {
      raw_data.main_util.cblack = raw_data.ifds[ifd].util.cblack;
    }
    /* END OF UTIL */

    /* APPLY OFFSETS */
    if (raw_data.ifds[ifd].meta_offset != 0) {
      raw_data.meta_offset = raw_data.ifds[ifd].meta_offset;
    }
    if (raw_data.ifds[ifd].tile_offset != 0) {
      raw_data.tile_offset = raw_data.ifds[ifd].tile_offset;
    }
    /* END OF OFFSETS */

    printf("\n");
  }



  return true;
}

bool RawImageData :: parse_raw(off_t raw_data_base) {
  // reset ifd count
  raw_data.ifd_count = 0; // fails since it shouldn't be 0 unless its the first call (in case of recursion)
  memset(raw_data.ifds, 0, sizeof(raw_data.ifds));
  file.seekg(0, std::ios::beg);
  if (!parse_raw_data(raw_data_base)) {
    return false;
  }

  return true;
}

bool RawImageData :: parse_raw_data(off_t raw_data_base) {
  // go to the base
  file.seekg(raw_data_base, std::ios::beg);
  
  u_int curr_bitorder, version;
  off_t ifd_offset;
  
  curr_bitorder = raw_data.bitorder;
  raw_data.bitorder = read_2_bytes_unsigned(file, raw_data.bitorder);
  
  if (raw_data.bitorder != 0x4949 && raw_data.bitorder != 0x4d4d) {
    raw_data.bitorder = curr_bitorder;
    return false;
  }

  version = read_2_bytes_unsigned(file, raw_data.bitorder);
  // ifd_offset = read_4_bytes_unsigned(file, raw_data.bitorder);

  while ((ifd_offset = read_4_bytes_unsigned(file, raw_data.bitorder))) {
    file.seekg(ifd_offset + raw_data_base, std::ios::beg);
    if (!parse_raw_data_ifd(raw_data_base)) {
      break;
    }
  }

  return true;
}

bool RawImageData :: parse_raw_data_ifd(off_t raw_data_base) {
  u_int ifd;
  if (raw_data.ifd_count >= sizeof(raw_data.ifds) / sizeof(raw_data.ifds[0])) {
    fprintf(stderr, "Raw File IFD Count Exceeded\n");
    return false;
  }
  raw_data.ifd_count++;
  ifd = raw_data.ifd_count;

  if (raw_data.ifds[ifd].set) {
    fprintf(stderr, "ERROR: Raw Image IFD Already SET\n");
    return false;
  }

  u_int n_tag_entries = read_2_bytes_unsigned(file, raw_data.bitorder);
  if (n_tag_entries != 0) {
    raw_data.ifds[ifd].set = true;
    raw_data.ifds[ifd].n_tag_entries = n_tag_entries;
  }

  for (u_int tag = 0; tag < raw_data.ifds[ifd].n_tag_entries; ++tag) {
    parse_raw_data_ifd_tag(ifd, raw_data_base);
  }

  return true;
}

void RawImageData :: parse_raw_data_ifd_tag(u_int ifd, off_t raw_data_base) {
  u_int tag_id, tag_type, tag_count;
  off_t offset, tag_data_offset, tag_offset, sub_ifd_offset;
  get_tag_header(raw_data_base, &tag_id, &tag_type, &tag_count, &tag_offset);
  printf("tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
  tag_data_offset = get_tag_data_offset(raw_data_base, tag_type, tag_count);
  
  file.seekg(tag_data_offset, std::ios::beg); // Jump to data offset
  switch(tag_id) {
    case 254: case 0:   // NewSubfileType
      break;
    case 255: case 1:   // SubfileType
      break;
    case 256: case 2:   // ImageWidth
      raw_data.ifds[ifd].frame.width = get_tag_value(tag_type);
      break;
    case 257: case 3:   // ImageLength
      raw_data.ifds[ifd].frame.height = get_tag_value(tag_type);
      break;
    case 258: case 4:   // BitsPerSample
      raw_data.ifds[ifd].frame.sample_pixel = tag_count;
      raw_data.ifds[ifd].frame.bps = get_tag_value(tag_type);
      break;
    case 259: case 5:   // Compression
      raw_data.ifds[ifd].frame.compression = get_tag_value(tag_type);
      break;
    case 262: case 8:   // PhotometricInterpretation
      raw_data.ifds[ifd].frame.pinterpret = get_tag_value(tag_type);
      break;
    case 271: case 17:  // Make
      file.read(raw_data.ifds[ifd].exif.camera_make, 64);
      break;
    case 272: case 18:  // Model
      file.read(raw_data.ifds[ifd].exif.camera_model, 64);
      break;
    case 273: case 19:  // StripOffsets
      raw_data.ifds[ifd].data_offset = get_tag_value(tag_type) + raw_data_base;
      parse_strip_data(ifd, raw_data_base);
      break;
    case 274: case 20:  // Orientation
      raw_data.ifds[ifd].frame.orientation = get_tag_value(tag_type);
      break;
    case 277: case 23:  // SamplesPerPixel
      raw_data.ifds[ifd].frame.sample_pixel = get_tag_value(tag_type);
      break;
    case 278: case 24:  // RowsPerStrip
      raw_data.ifds[ifd].rows_per_strip = get_tag_value(tag_type);
      break;
    case 279: case 25:  // StripByteCounts
      raw_data.ifds[ifd].strip_byte_counts = get_tag_value(tag_type);
      break;
    case 282: case 28:  // XResolution
      raw_data.ifds[ifd].frame.x_res = get_tag_value(tag_type);
      break;
    case 283: case 29:  // YResolution
      raw_data.ifds[ifd].frame.y_res = get_tag_value(tag_type);
      break;
    case 284: case 30:  // PlanarConfiguration
      raw_data.ifds[ifd].frame.planar_config = get_tag_value(tag_type);
      break;
    case 296: case 42:  // ResolutionUnit
      break;
    case 305: case 51:  // Software
      file.read(raw_data.ifds[ifd].exif.software, 64);
      break;
    case 306: case 52:  // DateTime
      parse_time_stamp(ifd);
      break;
    case 315: case 61:  // Artist
      file.read(raw_data.ifds[ifd].exif.artist, 64);
      break;
    case 320: case 66:  // ColorMap
      break;
    case 322: case 68:  // TileWidth
      raw_data.ifds[ifd].frame.tile_width = get_tag_value(tag_type); 
      break;
    case 323: case 69:  // TileLength
      raw_data.ifds[ifd].frame.tile_length = get_tag_value(tag_type);
      break;
    case 324: case 70:  // TileOffsets
      raw_data.ifds[ifd].tile_offset = get_tag_value(tag_type) + raw_data_base;
      break;
    case 325: case 71:  // TileByteCounts
      break;
    case 330: case 76:  // SubIFDs
      while (tag_count--) {
        offset = file.tellg();
        sub_ifd_offset = read_4_bytes_unsigned(file, raw_data.bitorder) + raw_data_base;
        file.seekg(sub_ifd_offset, std::ios::beg);
        if (!parse_raw_data_ifd(raw_data_base)) {
          break;
        }
        file.seekg(offset + 4, std::ios::beg);
      }
      break;
    case 513:           // JPEGInterchangeFormat
      raw_data.ifds[ifd].data_offset = get_tag_value(tag_type) + raw_data_base;
      parse_strip_data(ifd, raw_data_base);
      break;
    case 514:           // JPEGInterchangeFormatLength
      raw_data.ifds[ifd].jpeg_if_length = get_tag_value(tag_type);
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
      file.read(raw_data.ifds[ifd].exif.copyright, 64);
      break;
    case 33434:         // ExposureTime
      raw_data.ifds[ifd].exif.exposure = get_tag_value(tag_type);
      break;
    case 33437:         // FNumber
      raw_data.ifds[ifd].exif.f_number = get_tag_value(tag_type);
      break;
    case 34665:         // Exif IFD
      raw_data.ifds[ifd].exif.offset = get_tag_value(tag_type) + raw_data_base;
      parse_exif_data(ifd, raw_data_base);
      break;
    case 34675:         // InterColorProfile
      raw_data.ifds[ifd].exif.icc_profile_offset = file.tellg();
      raw_data.ifds[ifd].exif.icc_profile_count = tag_count;
      break;
    case 34853:         // GPSInfo / GPS IFD
      raw_data.ifds[ifd].exif.gps_offset = get_tag_value(tag_type) + raw_data_base;
      parse_gps_data(ifd, raw_data_base);
      break;
    case 37386:         // FocalLength
      raw_data.ifds[ifd].exif.focal_length = get_tag_value(tag_type);
      break;
    case 37393:         // ImageNumber
      raw_data.ifds[ifd].exif.image_count = get_tag_value(tag_type);
    case 46274:
      break;
    case 50706:         // DNGVersion
      // DNG
      break;
    case 50831:         // AsShotICCProfile
      raw_data.ifds[ifd].exif.icc_profile_offset = file.tellg();
      raw_data.ifds[ifd].exif.icc_profile_count = tag_count;
      break;
    
    default:
      break;
  }
  file.seekg(tag_offset, std::ios::beg);
}

bool RawImageData :: parse_strip_data(u_int ifd, off_t raw_data_base) {
  jpeg_info_t jpeg_info;
  if (raw_data.ifds[ifd].frame.bps || raw_data.ifds[ifd].data_offset == 0) {
    return false;
  }
  file.seekg(raw_data.ifds[ifd].data_offset, std::ios::beg);
  if (parse_jpeg_info(file, &jpeg_info, true)) {
    // print_jpeg_info(&jpeg_info);
    raw_data.ifds[ifd].frame.compression = 6;
    raw_data.ifds[ifd].frame.width = jpeg_info.width;
    raw_data.ifds[ifd].frame.height = jpeg_info.height;
    raw_data.ifds[ifd].frame.bps = jpeg_info.precision;
    raw_data.ifds[ifd].frame.sample_pixel = jpeg_info.components;

    parse_raw_data(raw_data.ifds[ifd].data_offset + 12);
  }
  return true;
}

bool RawImageData :: parse_exif_data(u_int ifd, off_t raw_data_base) {
  u_int n_tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;

  file.seekg(raw_data.ifds[ifd].exif.offset, std::ios::beg);
  n_tag_entries = read_2_bytes_unsigned(file, raw_data.bitorder);
  for (int i = 0; i < n_tag_entries; ++i) {
    get_tag_header(raw_data_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    printf("Exif tag: %d type: %d count: %d offset: %d\n", tag_id, tag_type, tag_count, tag_offset);
    tag_data_offset = get_tag_data_offset(raw_data_base, tag_type, tag_count);
    file.seekg(tag_data_offset, std::ios::beg);

    switch (tag_id) {
      case 0x829a:  // ExposureTime
        if (!raw_data.ifds[ifd].exif.exposure) {
          raw_data.ifds[ifd].exif.exposure = get_tag_value(tag_type);
        }
        break;
      case 0x829d:  // FNumber
        if (!raw_data.ifds[ifd].exif.f_number) {
          raw_data.ifds[ifd].exif.f_number = get_tag_value(tag_type);
        }
        break;
      case 0x8822:  // ExposureProgram
        break;
      case 0x8827:  // ISO
        if (!raw_data.ifds[ifd].exif.iso_sensitivity) {
          raw_data.ifds[ifd].exif.iso_sensitivity = get_tag_value(tag_type);
        }
        break;
      case 0x8833:  // ISOSpeed
      case 0x8834:  // ISOSpeedLatitudeyyy
        parse_time_stamp(ifd);
        break;
      case 0x9201:  // ShutterSpeedValue
        double exposure;
        if ((exposure = -get_tag_value(tag_type)) < 128) {
          raw_data.ifds[ifd].exif.exposure = pow(2, exposure);
        }
        break;
      case 0x9202:  // ApertureValue
        raw_data.ifds[ifd].exif.f_number = pow(2, get_tag_value(tag_type) / 2);
        break;
      case 0x920a:  // FocalLength
        raw_data.ifds[ifd].exif.focal_length = get_tag_value(tag_type);
        break;
      case 0x927c:  // MakerNote
        parse_makernote(ifd, raw_data_base, 0);
        break;
      case 0x9286:  // UserComment
        break;
      case 0xa302:  // CFAPattern
        if (read_4_bytes_unsigned(file, raw_data.bitorder) == 0x20002) {
          for (u_int i = 0; i < 8; i += 2) {
            raw_data.ifds[ifd].util.cfa = i;
            raw_data.ifds[ifd].util.cfa |= file.get() * BIT_MASK << i;
          }
        }
        break;
      default:
        break;
    }
    file.seekg(tag_offset, std::ios::beg); 
  }
  return true;
}

bool RawImageData :: parse_gps_data(u_int ifd, off_t raw_data_base) {
  u_int n_tag_entries, tag_id, tag_type, tag_count;
  off_t tag_data_offset, tag_offset;
  int data;
  file.seekg(raw_data.ifds[ifd].exif.gps_offset, std::ios::beg);
  n_tag_entries = read_2_bytes_unsigned(file, raw_data.bitorder);
  for (int i = 0; i < n_tag_entries; ++i) {
    get_tag_header(raw_data_base, &tag_id, &tag_type, &tag_count, &tag_offset);
    tag_data_offset = get_tag_data_offset(raw_data_base, tag_type, tag_count);
    file.seekg(tag_data_offset, std::ios::beg);
    switch (tag_id) {
      case 0:
        for (u_int i = 0; i < 4; ++i) {
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

bool RawImageData :: parse_time_stamp(u_int ifd) {
  // Proper date time format: " YYYY:MM:DD HH:MM:SS"
  file.read(raw_data.ifds[ifd].exif.date_time_str, 20);

  struct tm t;
  memset(&t, 0, sizeof(t));
  if (sscanf(raw_data.ifds[ifd].exif.date_time_str, "%d:%d:%d %d:%d:%d",
  &t.tm_year, &t.tm_mon, &t.tm_mday, 
  &t.tm_hour, &t.tm_min, &t.tm_sec) != 6) return false;

  t.tm_year -= 1900;
  t.tm_mon -= 1;
  t.tm_isdst = -1;

  if (mktime(&t) > 0) {
    raw_data.ifds[ifd].exif.date_time = mktime(&t);
  }

  return true;
}

off_t RawImageData :: get_tag_data_offset(off_t raw_data_base, u_int tag_type, u_int tag_count) {
  u_int type_byte = 0;
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
    return read_4_bytes_unsigned(file, raw_data.bitorder) + raw_data_base;;
  }
  return file.tellg();
}

void RawImageData :: get_tag_header(off_t raw_data_base, u_int *tag_id, u_int *tag_type, u_int *tag_count, off_t *tag_offset) {
  *tag_id = read_2_bytes_unsigned(file, raw_data.bitorder);
  *tag_type = read_2_bytes_unsigned(file, raw_data.bitorder);
  *tag_count = read_4_bytes_unsigned(file, raw_data.bitorder);
  *tag_offset = static_cast<int>(file.tellg()) + 4;
}

double RawImageData :: get_tag_value(u_int tag_type) {
  double numerator, denominator;
  char byte_array[8];
  int i, reverse_order;
  switch (tag_type) {
    case 1: // BYTE
      return read_1_byte_unsigned(file, raw_data.bitorder);
    case 2: // ASCII
      return read_1_byte_unsigned(file, raw_data.bitorder);
    case 3: // SHORT
      return read_2_bytes_unsigned(file, raw_data.bitorder);
    case 4: // LONG
      return read_4_bytes_unsigned(file, raw_data.bitorder);
    case 5: // RATIONAL
      numerator = read_4_bytes_unsigned(file, raw_data.bitorder);
      denominator = read_4_bytes_unsigned(file, raw_data.bitorder);
      return numerator / denominator;
    case 6: // SBYTE
      return read_1_byte_signed(file, raw_data.bitorder);
    case 7: // UNDEFINED
      return read_1_byte_unsigned(file, raw_data.bitorder);
    case 8: // SSHORT
      return read_2_byte_signed(file, raw_data.bitorder);
    case 9: // SLONG
      return read_4_byte_signed(file, raw_data.bitorder);
    case 10:// SRATIONAL
      numerator = read_4_byte_signed(file, raw_data.bitorder);
      denominator = read_4_byte_signed(file, raw_data.bitorder);
      return numerator / denominator;
    case 11://FLOAT
      return *reinterpret_cast<float*>(read_4_bytes_unsigned(file, raw_data.bitorder));
    case 12://DOUBLE
      reverse_order = 7 * ((raw_data.bitorder == 0x4949) == (ntohs(0x1234) == 0x1234));
      for (i = 0; i < 8; i++) {
        int index = i ^ reverse_order;
        byte_array[index] = read_1_byte_unsigned(file, raw_data.bitorder);
      }
      double value;
      memcpy(&value, byte_array, sizeof(value));
      return value;

    default:
      return read_1_byte_signed(file, raw_data.bitorder);
  }
}


void RawImageData :: print_data(bool rawFileData, bool exifData, bool rawTiffIfds) {
  if (!rawFileData) goto exif;
  printf("\n================RAW FILE DATA===============\n");
  printf("Base offset: %d\n", raw_data.base);
  printf("Bit ordering: 0x%x\n", raw_data.bitorder);
  printf("Version: %d\n", raw_data.version);
  printf("File size: %.2lfMB\n", (float)raw_data.file_size / 1000000);

  printf("Frame:\n");
  printf("Width: %d\n", raw_data.main_frame.width);
  printf("Height: %d\n", raw_data.main_frame.height);
  printf("BPS: %d\n", raw_data.main_frame.bps);
  printf("Compression: %d\n", raw_data.main_frame.compression);
  printf("Sample per pixel: %d\n", raw_data.main_frame.sample_pixel);
  printf("CFA Pattern: %d\n", raw_data.main_util.cfa);
  printf("Camera White Balance Multiplier (RGB): %lf %lf %lf\n", raw_data.main_util.white_balance_multi_cam.r, raw_data.main_util.white_balance_multi_cam.g, raw_data.main_util.white_balance_multi_cam.b);
  printf("Cblack (RGGB): %d %d %d %d\n", raw_data.main_util.cblack.r, raw_data.main_util.cblack.g_r, raw_data.main_util.cblack.g_b, raw_data.main_util.cblack.b);

  exif:
    if (!exifData) goto rawifd;
    printf("\n================EXIF DATA===============\n");
    printf("Camera Make: %s\n", raw_data.main_exif.camera_make);
    printf("Camera Model: %s\n", raw_data.main_exif.camera_model);
    printf("Software: %s\n", raw_data.main_exif.software);
  
    printf("Focal Length: %lf\n", raw_data.main_exif.focal_length);
    printf("Exposure: %lf\n", raw_data.main_exif.exposure);
    printf("F Number: %lf\n", raw_data.main_exif.f_number);
    printf("ISO Sensitivity: %lf\n", raw_data.main_exif.iso_sensitivity);
  
    printf("Lens Model: %s\n", raw_data.main_exif.lens_info.lens_model);
    printf("Lens Type: %d\n", raw_data.main_exif.lens_info.lens_type);
    printf("Lens Focal Length: Min %lf, Max %lf\n", raw_data.main_exif.lens_info.min_focal_length, raw_data.main_exif.lens_info.max_focal_length);
    printf("Lens Aperture F: Min %lf, Max %lf\n", raw_data.main_exif.lens_info.min_f_number, raw_data.main_exif.lens_info.max_f_number);

    printf("Image Count: %d\n", raw_data.main_exif.image_count);
    printf("Shutter Count: %d\n", raw_data.main_exif.shutter_count);

    printf("Artist: %s\n", raw_data.main_exif.artist);
    printf("Copyright: %s\n", raw_data.main_exif.copyright);
    printf("Date time: %s\n", raw_data.main_exif.date_time_str);

  rawifd:
    if (!rawTiffIfds) goto end;
    printf("\n================RAW TIFF IFDs===============\n");
    for (u_int i = 0; i < 8; ++i) {
      if (raw_data.ifds[i].set) {
        printf("IFD: %d\n", i + 1);
        printf("N Tag Entries: %d\n", raw_data.ifds[i].n_tag_entries);
        printf("Width: %d\n", raw_data.ifds[i].frame.width);
        printf("Height: %d\n", raw_data.ifds[i].frame.height);
        printf("BPS: %d\n", raw_data.ifds[i].frame.bps);
        printf("Compression: %d\n", raw_data.ifds[i].frame.compression);
        printf("P Interpret: %d\n", raw_data.ifds[i].frame.pinterpret);
        printf("Orientation: %d\n", raw_data.ifds[i].frame.orientation);
        printf("Offset: %d\n", raw_data.ifds[i].data_offset);
        printf("Tile Offset: %d\n", raw_data.ifds[i].tile_offset);
        printf("\n");
      }
    }

  end:
    return;
}
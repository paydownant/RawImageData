
#include "jpegimagedata.h"

/**
 * https://yasoob.me/posts/understanding-and-writing-jpeg-decoder-in-python/
 */

bool parse_jpeg_info(std::ifstream& file, jpeg_info_t* jpeg_info, bool info_only) {

  u_char buffer[2];  
  if (!file.read(reinterpret_cast<char*>(&buffer), 2)) {
    return false;
  }
  if (memcmp(buffer, MAGIC_JPEG, 2)) {
    // Magic number does not match FF D8 ...
    fprintf(stderr, "ERROR: check jpeg: %02X %02X\n", buffer[0], buffer[1]);
    return false;
  }

  u_short marker, length;
  u_char marker_length[4], data[0xfffff];
  const u_char *dp;
  bool c_sof = false, c_dht = false, c_sos = false, c_dqt = false, c_dri = false;
  while (file.read(reinterpret_cast<char*>(&marker_length), 4)) {
    if (marker_length[0] != 0xff) {
      continue;
    }
    marker = (marker_length[0] << 8 | marker_length[1]);
    length = (marker_length[2] << 8 | marker_length[3]) - 2;
    
    file.read(reinterpret_cast<char*>(&data), length);
    dp = data;
    printf("JPEG Marker: 0x%x, length: %d\n", marker, length);
    switch (marker) {
      case 0xffe0:  // APP0 (Application 0)
        break;
      case 0xffc0:  // SOF0 (Start of Frame)
      case 0xffc1:  // SOF1 (Start of Frame)
        c_sof = parse_sof(jpeg_info, dp, marker, length);
        break;
      case 0xffc4:  // DHF (Define Huffman Table)
        if (info_only) break;
        c_dht = parse_dht(jpeg_info, dp, marker, length);
        break;
      case 0xffda:  // SOS (Start of Scan)
        c_sos = parse_sos(jpeg_info, dp, marker, length);
        break;
      case 0xffdb:  // DQT (Define Quantisation Table)
        c_dqt = parse_dqt(jpeg_info, dp, marker, length);
        break;
      case 0xffdd:  // DRI (Define Restart Interval)
        c_dri = parse_dri(jpeg_info, dp, marker, length);
        break;
    
      default:  // skip
        file.seekg(length, std::ios_base::cur);
        break;
    }
    if (marker == 0xffda) {
      break;
    }
  }

  if (!c_sof) {
    return false;
  }
  if (info_only) {
    return true;
  }
  if (!c_dht || !c_sos || !c_dqt || !c_dri) {
    printf("ERROR: JPEG HEADER NOT FULL");
    return false;
  }

  return true;
}

bool parse_sof(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length) {
  off_t offset = 0;
  if (jpeg_info->components != 0) {
    return false;
  }
  jpeg_info->frame_type = marker & 0xff;
  jpeg_info->precision = data[offset++];
  jpeg_info->height = (data[offset++] << 8 | data[offset++]);
  jpeg_info->width = (data[offset++] << 8 | data[offset++]);
  jpeg_info->components = data[offset++];

  colour_component_t *component;
  u_int component_id, sampling_factor;
  for (u_int i = 0; i < jpeg_info->components; ++i) {
    component_id = data[offset++];
    if (component_id == 0) {
      jpeg_info->zero_based = true;
    }
    if (jpeg_info->zero_based) {
      component_id += 1;
    }
    if (component_id == 0 || component_id > 5) {
      return false;
    }

    component = &jpeg_info->colour_components[component_id - 1];
    if (component->set) {
      return false;
    }
    component->set = true;
    sampling_factor = data[offset++];
    component->h_sampling_factor = sampling_factor >> 4;
    component->v_sampling_factor = sampling_factor & 0x0f;
    component->quantisation_table_id = data[offset++];
    if (component->quantisation_table_id > 3) {
      return false;
    }
  }

  if (jpeg_info->precision > 16 || jpeg_info->precision == 0) {
    return false;
  }
  if (jpeg_info->components > 6 || jpeg_info->components == 0) {
    return false;
  }
  if (jpeg_info->height == 0 || jpeg_info->width == 0) {
    return false;
  }

  return true;
}

bool parse_dqt(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length) {
  off_t offset = 0;
  u_int8_t table_info, table_id, table_class;
  
  while (offset < length) {
    table_info = data[offset];
    offset++;
    table_class = (table_info << 4) & 0x0f;
    table_id = table_info & 0x0f;

    if (table_id > 3) {
      fprintf(stderr, "ERROR: Invalid quantisation table ID of %d\n", (u_int)table_id);
      return false;
    }
    jpeg_info->quant[table_id].set = true;
    
    if (table_class != 0) {
      for (u_int i = 0; i < 64; ++i) {
        jpeg_info->quant[table_id].table[ZZ_MATRIX[i]] = (data[offset++] << 8) + (data[offset++]);
      }
    } else {
      for (u_int i = 0; i < 64; ++i) {
        jpeg_info->quant[table_id].table[ZZ_MATRIX[i]] = (data[offset++]);
      }
    }
  }
  return true;
}

bool parse_dht(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length) {
  off_t offset = 0;
  u_int8_t table_info, table_class, table_id;
  while (offset < length) {
    table_info = data[offset++];
    table_class = (table_info << 4) & 0x0f;  // 0: DC, 1: AC
    table_id = table_info & 0x0f;            // Table ID (0-3)
    if (table_id > 3) {
      fprintf(stderr, "ERROR: DHT invalid ID: %d\n", table_id);
      return false;
    }

    huff_table_t *huff_table;
    if (table_class == 0) {
      /* DC Table */
      huff_table = &jpeg_info->huff_dc_tables[table_id];
    } else if (table_class == 1) {
      /* AC Table */
      huff_table = &jpeg_info->huff_ac_tables[table_id];
    } else {
      fprintf(stderr, "ERROR: DHT invalid class: %d\n", table_class);
      return false;
    }
    huff_table->set = true;

    huff_table->offsets[0] = 0;
    u_int num_symbols = 0;
    for (u_int i = 1; i <= 16; ++i) {
      num_symbols += data[offset++];
      huff_table->offsets[i] = num_symbols;
    }

    if (num_symbols > 162) {
      fprintf(stderr, "ERROR: Number of Huff Symbols Exceeded 162: %d\n", num_symbols);
      return false;
    }
    for (u_int i = 0; i < num_symbols; ++i) {
      huff_table->symbols[i] = data[offset++];
    }
  }
  return true;
}

bool parse_sos(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length) {
  off_t offset = 0;
  u_int num_components, component_id;
  colour_component_t *component;

  if (jpeg_info->components == 0) {
    fprintf(stderr, "ERROR: SOS was read before SOF\n");
    return false;
  }
  
  for (u_int i = 0; i < jpeg_info->components; ++i) {
    jpeg_info->colour_components[i].set = false;
  }

  num_components = data[offset++];
  for (u_int i = 0; i < num_components; ++i) {
    component_id = data[offset++];
    if (jpeg_info->zero_based) {
      component_id += 1;
    }
    if (component_id > jpeg_info->components) {
      fprintf(stderr, "ERROR: Component ID\n");
      return false;
    }
    
    component = &jpeg_info->colour_components[component_id];
    if (component->set) {
      fprintf(stderr, "ERROR: Duplicate colour component ID\n");
      return false;
    }
    component->set = true;

    u_int huff_table_id = data[offset++];
    component->huff_dc_table_id = huff_table_id >> 4;
    component->huff_ac_table_id = huff_table_id & 0x0f;

    if (component->huff_dc_table_id > 3 || component->huff_ac_table_id > 3) {
      fprintf(stderr, "ERROR: Huffman Table ID out of bounds\n");
      return false;
    }
  }

  jpeg_info->start_selection = data[offset++];
  jpeg_info->end_selection = data[offset++];
  u_int successive_approximation = data[offset++];
  jpeg_info->s_approx_high = successive_approximation >> 4;
  jpeg_info->s_approx_low = successive_approximation & 0x0f;

  if (jpeg_info->start_selection != 0 || jpeg_info->end_selection != 63) {
    return false;
  }
  if (jpeg_info->s_approx_high != 0 || jpeg_info->s_approx_low != 0) {
    return false;
  }
  if (offset != length) {
    return false;
  }

  return true;
}

bool parse_dri(jpeg_info_t* jpeg_info, const u_char* data, const u_int marker, const u_int length) {
  off_t offset = 0;
  jpeg_info->restart_interval = (data[offset++] << 8 | data[offset++]);
  return true;
}

void print_jpeg_info(jpeg_info_t* jpeg_info) {
  printf("JPEG_INFO\n");

  printf("================SOF================\n");
  printf("Frame Type: 0x%x\n", jpeg_info->frame_type);
  printf("Precision: %d\n", jpeg_info->precision);
  printf("Width: %d\n", jpeg_info->width);
  printf("Height: %d\n", jpeg_info->height);
  printf("Components: %d\n", jpeg_info->components);
  for (u_int i = 0; i < 6; ++i) {
    if (jpeg_info->colour_components[i].set) {
      printf("  Colour Component:\n");
      printf("  Horizontal Sampling Factor: %d\n", jpeg_info->colour_components[i].h_sampling_factor);
      printf("  Vertical Sampling Factor: %d\n", jpeg_info->colour_components[i].v_sampling_factor);
      printf("  Quantisation Table ID: %d\n", jpeg_info->colour_components[i].quantisation_table_id);
    }
  }
  printf("===================================\n");

  printf("================DQT================\n");
  for (u_int i = 0; i < 4; ++i) {
    if (jpeg_info->quant[i].set) {
      for (u_int j = 0; j < 64; ++j) {
        printf("%d ", jpeg_info->quant[i].table[j]);
        if ((j+1) % 8 == 0) {
          printf("\n");
        }
      }
      printf("\n");
    }
  }
  printf("===================================\n");

  printf("================DHT================\n");
  printf("DC Tables:\n");
  for (u_int i = 0; i < 4; ++i) {
    if (jpeg_info->huff_dc_tables[i].set) {
      printf("Table ID: %d\n", i);
      printf("Symbols:\n");
      for (u_int j = 0; j < 16; ++j) {
        printf("%d: ", j + 1);
        for (u_int k = jpeg_info->huff_dc_tables[i].offsets[j]; k < jpeg_info->huff_dc_tables[i].offsets[j + 1]; ++k) {
          printf("%x ", (u_int)jpeg_info->huff_dc_tables[i].symbols[k]);
        }
        printf("\n");
      }
    } 
  }
  printf("\n");
  printf("AC Tables:\n");
  for (u_int i = 0; i < 4; ++i) {
    if (jpeg_info->huff_ac_tables[i].set) {
      printf("Table ID: %d\n", i);
      printf("Symbols:\n");
      for (u_int j = 0; j < 16; ++j) {
        printf("%d: ", j + 1);
        for (u_int k = jpeg_info->huff_ac_tables[i].offsets[j]; k < jpeg_info->huff_ac_tables[i].offsets[j + 1]; ++k) {
          printf("%x ", (u_int)jpeg_info->huff_ac_tables[i].symbols[k]);
        }
        printf("\n");
      }
    } 
  }
  printf("===================================\n");

  printf("================SOS================\n");
  printf("Start of Selection: %d\n", jpeg_info->start_selection);
  printf("End of Selection: %d\n", jpeg_info->end_selection);
  printf("Successive Approximation High: %d\n", jpeg_info->s_approx_high);
  printf("Successive Approximation Low: %d\n", jpeg_info->s_approx_low);
  printf("Colour Components:\n");
  for (u_int i = 0; i < jpeg_info->components; ++i) {
    printf("Component ID: %d\n", i + 1);
    printf("Huffman DC Table ID: %d\n", (u_int)jpeg_info->colour_components[i].huff_dc_table_id);
    printf("Huffman AC Table ID: %d\n", (u_int)jpeg_info->colour_components[i].huff_ac_table_id);
  }
  printf("Length of Huffman Data: %d\n", jpeg_info->huff_data.size());
  
  printf("===================================\n");

  printf("================DRI================\n");
  printf("Restart Interval: %d\n", jpeg_info->restart_interval);
  printf("===================================\n");
}
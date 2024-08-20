#ifndef NIKON_RAW_H
#define NIKON_RAW_H

#include "../rawimagedata.h"

class NikonRaw : public RawImageData {

public:
  NikonRaw(const std::string& filepath);
  ~NikonRaw();

private:
  /* Overrides */
  bool load_raw_data() override;
  bool parse_makernote(u_int ifd, off_t raw_data_base, int uptag) override;


  /* Unique Functinos */
  void parse_markernote_tag(u_int ifd, off_t raw_data_base, int uptag);

  
  /* Variables */
  const u_int nikon_huff_tree[6][32] = {
    {0},
    {0},
    {0},
    {0},
    {0},
    {0}
  };

};

#endif
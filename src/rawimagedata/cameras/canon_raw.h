#ifndef CANON_RAW_H
#define CANON_RAW_H

#include "../rawimagedata.h"

class CanonRaw : public RawImageData {

public:
  CanonRaw(const std::string& filepath);
  ~CanonRaw();

  bool load_raw_data() override;
  
private:
  /* Overrides */
  bool parse_makernote(u_int ifd, off_t raw_data_base, int uptag) override;

  /* Unique Functinos */
  void parse_markernote_tag(u_int ifd, off_t raw_data_base, int uptag);

};

#endif
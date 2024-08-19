#ifndef CANON_RAW_H
#define CANON_RAW_H

#include "../rawimagedata.h"

class CanonRaw : public RawImageData {

public:
  /* Public Functions */
  CanonRaw(const std::string& filepath);
  ~CanonRaw();

protected:
  /* Protected Functions */
  bool parse_makernote(u_int ifd, off_t raw_data_base, int uptag) override;
  void parse_markernote_tag(u_int ifd, off_t raw_data_base, int uptag) override;

};

#endif
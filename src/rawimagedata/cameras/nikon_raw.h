#ifndef NIKON_RAW_H
#define NIKON_RAW_H

#include "../rawimagedata.h"

class NikonRaw : public RawImageData {

public:
  /* Public Functions */
  NikonRaw(const std::string& filepath);
  ~NikonRaw();

protected:
  /* Protected Functions */
  bool parse_makernote(off_t raw_data_base, int uptag) override;
  void parse_markernote_tag(off_t raw_data_base, int uptag) override;

};

#endif
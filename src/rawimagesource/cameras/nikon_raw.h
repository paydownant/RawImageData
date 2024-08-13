
#include "../rawimagedata.h"

class NikonRaw : public RawImageData {

public:
  NikonRaw(const std::string& filepath);

  bool parse_makernote(off_t raw_image_file_base, int uptag) override;
  void parse_markernote_tag(off_t raw_image_file_base, int uptag) override;

};

#include "rawimagedata.h"

class RawImageData_Nikon : public RawImageData {

public:
  RawImageData_Nikon(const std::string& filepath);

  bool parse_makernote(off_t raw_image_file_base, int uptag) override;
  void parse_markernote_tag(off_t raw_image_file_base, int uptag) override;

};
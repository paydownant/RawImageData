
#include <stdio.h>
#include <stdlib.h>

#include "rawimagedata_nikon.h"

int main(int argc, char** argv) {
  RawImageData_Nikon img("../sample_images/nikon/DSC_0498.NEF");
  img.load_raw();
  // RawImageData img("../sample_images/canon/canon-eos-r.cr3");
}
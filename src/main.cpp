
#include <stdio.h>
#include <stdlib.h>

#include "rawimagesource/cameras/nikon_raw.h"

int main(int argc, char** argv) {
  NikonRaw img("../sample_images/nikon/DSC_0498.NEF");
  img.load_raw();
  // RawImageData img("../sample_images/canon/canon-eos-r.cr3");
}
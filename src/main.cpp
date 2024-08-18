
#include <stdio.h>
#include <stdlib.h>

#include "rawimagedata/cameras/nikon_raw.h"
#include "rawimagedata/cameras/canon_raw.h"

int main(int argc, char** argv) {
  RawImageData *img;
  int index = 0;

  if (index == 0) {
    img = new NikonRaw("../sample_images/nikon/DSC_0498.NEF");
  } else if (index == 1) {
    img = new CanonRaw("../sample_images/canon/canon-eos-r.cr3");
  } else {
    /* Some other child classes*/
  }
  
  if (img != nullptr) {
    img->load_raw();
  }
  
}
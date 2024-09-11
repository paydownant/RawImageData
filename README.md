# Camera Raw File Parser

This C++ project is a simple parser for camera raw files. It reads raw image data from a file, identifies the raw image format, and processes it. The main entry point for the program is the `RawImageData` class.

## Features

- **Raw File Identification**: Automatically identifies the raw file format.
- **Parsing**: Extracts raw image data from the file.
- **TIFF Application**: Applies raw frame data, typically in TIFF format.

## Installation

To build the project, you'll need a C++ compiler that supports C++11 or later. Follow these steps to compile the project:

1. Clone the repository:
   ```sh
   git clone https://github.com/paydownant/RawImageData.git
   cd RawImageData
   mkdir build
   cd build
   ```

2. Build the project using `cmake` or any other build system:
   ```sh
   cmake ..
   ```

3. Run:
   ```sh
   ./image
   ```

## Usage

To use the Camera Raw File Parser, create an instance of the `RawImageData`'s child camera class, in the example, `NikonRaw` class, and provide the path to the raw file you want to parse.

### Example

```cpp
#include <stdio.h>
#include <stdlib.h>

#include "rawimagedata/cameras/nikon_raw.h"

int main(int argc, char** argv) {
  RawImageData *img;

  img = new NikonRaw("../sample_images/nikon/DSC_1551.NEF");
  
  if (img != nullptr) {
    img->load_raw();
  }
  
  delete img;
}
```

### Entry Point

The main entry point for the program is the constructor of the `RawImageData` class:

```cpp
RawImageData :: RawImageData(const std::string& file_path) : file_path(file_path), file(file_path, std::ios::binary) {
  if (!file) {
    throw std::runtime_error("Unable to open file: " + file_path);
  }
}
```

### Description

- **Constructor (`RawImageData`)**: 
  - Takes a `file_path` as a string argument.
  - Opens the file in binary mode.
  - Throws an exception if the file cannot be opened.
  - Calls `raw_identify()` to identify the raw image format.
  - If the raw image is successfully parsed via `parse_raw()`, it prints the image data and applies the raw frame, usually in TIFF format.

## Contact

If you have any questions or suggestions, feel free to open an issue or contact the project maintainers.

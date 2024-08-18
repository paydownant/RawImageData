cmake_minimum_required(VERSION 3.8)

project(image)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/rawimagesource/rawimagedata.cpp
  src/rawimagesource/rawimagedata_utils.cpp
  src/rawimagesource/jpegimagedata.cpp

  src/rawimagesource/cameras/nikon_raw.cpp
)
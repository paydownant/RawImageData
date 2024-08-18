cmake_minimum_required(VERSION 3.8)

project(image)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/rawimagedata/rawimagedata.cpp
  src/rawimagedata/rawimagedata_utils.cpp
  src/rawimagedata/jpegimagedata.cpp

  src/rawimagedata/cameras/nikon_raw.cpp
)
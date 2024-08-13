cmake_minimum_required(VERSION 3.8)

project(image)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/rawimagedata.cpp
  src/rawimagedata_utils.cpp
  src/jpegimagedata.cpp

  src/rawimagedata_nikon.cpp
)
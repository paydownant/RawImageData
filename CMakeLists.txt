cmake_minimum_required(VERSION 3.8)

project(image)

# List camera raws
file(GLOB CAMERA_RAW_SOURCES src/rawimagedata/cameras/*.cpp)

add_executable(${PROJECT_NAME}
  src/main.cpp
  
  src/rawimagedata/rawimagedata.cpp
  src/rawimagedata/rawimagedata_utils.cpp

  src/rawimagedata/jpegimagedata.cpp

  ${CAMERA_RAW_SOURCES}
)
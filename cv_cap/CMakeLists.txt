cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
find_package( OpenCV REQUIRED )

find_package(PkgConfig)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp>=0.5)
include_directories(${OpenCV_INCLUDE_DIRS} ${YAMLCPP_INCLUDE_DIRS})

add_executable(DisplayImage DisplayImage.cpp)
add_executable(Detect detect.cpp)
add_executable(test_serial test_serial.cpp)
add_library(serial_com serial_com.cpp)

target_link_libraries(DisplayImage serial_com ${OpenCV_LIBS} ${YAMLCPP_LIBRARIES})
target_link_libraries(Detect ${OpenCV_LIBS} ${YAMLCPP_LIBRARIES})
target_link_libraries(test_serial serial_com)

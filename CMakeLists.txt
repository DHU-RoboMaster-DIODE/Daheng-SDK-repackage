cmake_minimum_required(VERSION 3.5)
PROJECT(Daheng)

SET(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3 -pthread")

find_package(OpenCV 3.0 QUIET)
include_directories("/home/domino/Downloads/Galaxy_1.2.1911.9122/Galaxy_camera/inc")

add_executable(Daheng main.cpp mainPC.cpp video.cpp)
target_link_libraries(Daheng -lgxiapi ${OpenCV_LIBS})

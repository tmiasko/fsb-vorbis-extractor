cmake_minimum_required(VERSION 3.0.2)

project (fsb-vorbis-extractor)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Ogg REQUIRED ogg)
pkg_check_modules(Vorbis REQUIRED vorbisenc)

enable_testing()

include_directories(./ ${Ogg_INCLUDE_DIRS} ${Vorbis_INCLUDE_DIRS})
link_directories(./ ${Ogg_LIBRARY_DIRS} ${Vorbis_LIBRARY_DIRS})

add_subdirectory(/usr/src/gtest/ gtest)
add_subdirectory(src)

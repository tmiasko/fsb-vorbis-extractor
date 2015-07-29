cmake_minimum_required(VERSION 3.0.2)

project (fsb-vorbis-extractor)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Ogg REQUIRED ogg)
pkg_check_modules(Vorbis REQUIRED vorbis vorbisenc)
pkg_check_modules(GLog REQUIRED libglog)

set (GTEST_ROOT "/usr/src/gtest/")
if(EXISTS ${GTEST_ROOT})
  option (FVE_BUILD_TESTS "Build fsb-vorbis-extractor tests." ON)
else()
  message(WARNING "Google test library not found, use GTEST_ROOT to specifiy its location")
endif()

if (FVE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(${GTEST_ROOT} gtest)
endif ()

include_directories(./ ${Ogg_INCLUDE_DIRS} ${Vorbis_INCLUDE_DIRS} ${GLog_INCLUDE_DIRS})
link_directories(./ ${Ogg_LIBRARY_DIRS} ${Vorbis_LIBRARY_DIRS} ${GLog_LIBRARY_DIRS})

add_subdirectory(src)


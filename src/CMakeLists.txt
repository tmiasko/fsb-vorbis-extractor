include_directories(./)

add_library(fsb STATIC
  fsb/io/buffer_view.cpp
  fsb/io/buffer_view.hpp
  fsb/io/filter.cpp
  fsb/io/filter.hpp
  fsb/io/utility.cpp
  fsb/io/utility.hpp
  fsb/vorbis/headers_generator.cpp
  fsb/vorbis/headers_generator.hpp
  fsb/vorbis/rebuilder.cpp
  fsb/vorbis/rebuilder.hpp
  fsb/vorbis/vorbis.cpp
  fsb/vorbis/vorbis.hpp
  fsb/container.cpp
  fsb/container.hpp
  fsb/fsb.hpp)
target_link_libraries(fsb
  glog
  ${Ogg_LIBRARIES}
  ${Vorbis_LIBRARIES})

add_executable(extractor
  fsb/extractor.cpp)
target_link_libraries(extractor
  fsb
  glog
  gflags
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

add_executable(headers_generator_tool
  fsb/vorbis/headers_generator_tool.cpp)
target_link_libraries(headers_generator_tool
  fsb
  glog
  ${Vorbis_LIBRARIES})

add_executable(fsb_test
  fsb/io/filter_test.cpp
  fsb/io/utility_test.cpp
  fsb/vorbis/headers_generator_test.cpp
  fsb/vorbis/rebuilder_test.cpp
  fsb/vorbis/vorbis_test.cpp)
add_test(fsb_test fsb_test)
target_link_libraries(fsb_test
  fsb
  glog
  gtest gtest_main)

// Copyright (C) 2015 Tomasz Miąsko
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
#include "headers_generator.hpp"

#include <boost/io/ios_state.hpp>
#include <glog/logging.h>

#include <iostream>
#include <iomanip>
#include <map>

using namespace fsb::vorbis;

namespace {

// Information necessary to reconstruct all Vorbis headers, that is not already
// found in FSB metadata.
struct headers_info {
  int blocksize_short;
  int blocksize_long;
  std::vector<unsigned char> setup_header;
};

// Compares all members of headers_info.
inline bool operator==(const headers_info &a, const headers_info &b) {
  return a.blocksize_short == b.blocksize_short
      && a.blocksize_long  == b.blocksize_long
      && a.setup_header    == b.setup_header;
}

// Formats bytes vector as C string literal.
void output_string_literal(
  std::ostream & os,const std::vector<unsigned char> & bytes) {
  
  // Res
  boost::io::ios_all_saver stream_state_saver(os);
  
  const std::size_t max_width = 79;
  std::size_t width = 3;
  os << "  \"";
  
  for (const auto byte : bytes) {
    if (width >= max_width) {
      // Reached max width, end current line and start a new one.
      os << "\"\n  \"";
      width = 3;
    }
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << int(byte);
    width += 4;
  }
  
  os << "\"\n";
}
  
}

int main(int argc, char **argv) {
  // Utility program that is used to generate vorbis_headers.inc file,
  // that contains Vorbis headers for all settings combinations used by FSB:
  // * quality: 1, 2, ..., 100
  // * channels: 1, 2
  // * rates:  
  const int rates[] {8000, 11000, 16000, 22050, 24000, 32000, 44100, 48000};
  // Not all of them result in unique Vorbis headers. 
  // Duplicates will be removed when populating headers map.
  std::map<uint32_t, headers_info> headers;
  
  for (int quality=1; quality <= 100; ++quality) {
    for (int channels=1; channels <= 2; ++channels) {
      for (const auto rate : rates) {
        const headers_generator generator(channels, rate, quality);
        const uint32_t key = crc32(generator.setup_header());
        
        vorbis_info * vi = const_cast<vorbis_info*>(&generator.info());
        const int blocksize_short = vorbis_info_blocksize(vi, 0);
        CHECK(blocksize_short != -1) << "vorbis_info_blocksize failed.";

        const int blocksize_long = vorbis_info_blocksize(vi, 1);
        CHECK(blocksize_long != -1) << "vorbis_info_blocksize failed.";
        
        const headers_info info {
          blocksize_short,
          blocksize_long,
          { 
            generator.setup_header().packet, 
            generator.setup_header().packet + generator.setup_header().bytes
          }
        };
        
        auto inserted = headers.emplace(key, info);
        if (!inserted.second) {
          // Sanity check that CRC-32 uniquely identifies Vorbis headers.
          CHECK(info == inserted.first->second) 
            << "Different headers with the same CRC.";
        }
      }
    }
  }
  
  std::cout
    << "// Generated automatically using headers_generator_tool.cpp"
    << std::endl;
  for (const auto & header : headers) {
    std::cout
      << "{\n"
      << "  " << header.first << ",\n" 
      << "  " << header.second.blocksize_short << ",\n"
      << "  " << header.second.blocksize_long << ",\n"
      << "  " << header.second.setup_header.size() << ",\n";
    output_string_literal(std::cout, header.second.setup_header);
    std::cout
      << "},\n";
  }
  
  return 0;
}
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
#ifndef FSB_CONTAINER_HPP
#define FSB_CONTAINER_HPP

#include "fsb/fsb.hpp"
#include "fsb/io/buffer_view.hpp"

#include <boost/utility/string_ref.hpp>

#include <iosfwd>
#include <vector>

namespace fsb {

class container {
  container(const container &) = delete;
  container & operator=(const container &) = delete;
  
public:
  container(std::istream & encoded_stream, boost::string_ref password);
  
  const header & file_header() const {
    return header_;
  }
  
  const std::vector<sample> & samples() const {
    return samples_;
  }
  
  // Extracts sample audio data to given stream.
  void extract_sample(const sample & sample, std::ostream & stream);
  
private:
  // Reads file header from a stream.
  void read_file_header(std::istream & stream);
  
  // Reads sample headers from a stream.
  void read_sample_headers(std::istream & stream);
  
  // Reads a single sample header from a view.
  // 
  // Position view on a next sample header.
  sample read_sample_header(io::buffer_view & view);
  
  // Reads sample names.
  void read_sample_names(std::istream & stream);
  
private:
  static const int header_size = 60;
  header header_;
  std::vector<sample> samples_;
  std::vector<char> data_buffer_;
};

}

#endif
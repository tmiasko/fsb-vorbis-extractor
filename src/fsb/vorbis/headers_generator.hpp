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
#ifndef FSB_VORBIS_HEADERS_GENERATOR_H
#define FSB_VORBIS_HEADERS_GENERATOR_H

#include "fsb/vorbis/vorbis.hpp"

#include <cstdint>

namespace fsb { namespace vorbis {

// Generates Vorbis headers using given codec settings. 
// Quality should be an integer in range [0, 100].
//
// This is a class instead of simple function to support retrieving additional
// information for testing purposes.
class headers_generator {
  headers_generator(const headers_generator &) = delete;
  headers_generator & operator=(const headers_generator &) = delete;
public:
  headers_generator(int channels, int rate, int quality);
  ~headers_generator();

  // Returns Vorbis info.
  const vorbis_info & info() const { return info_; }
  // Returns identification header.
  const ogg_packet & id_header() const { return header_id_; }
  // Returns comment header.
  const ogg_packet & comment_header() const { return header_comment_; }
  // Returns setup header.
  const ogg_packet & setup_header() const { return header_setup_; }

private:
  mutable vorbis_info_holder info_;
  vorbis_comment_holder comment_;
  vorbis_dsp_state dsp_state_;
  ogg_packet header_id_;
  ogg_packet header_comment_;
  ogg_packet header_setup_;
};
  
}}

#endif

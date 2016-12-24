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
#ifndef FSB_VORBIS_REBUILDER_HPP
#define FSB_VORBIS_REBUILDER_HPP

#include "fsb/io/buffer_view.hpp"
#include "fsb/vorbis/vorbis.hpp"

#include <cstdint>
#include <iosfwd>

namespace fsb { namespace vorbis {

// Rebuilds Vorbis headers and audio data.
class rebuilder {
  rebuilder(const rebuilder &) = delete;
  rebuilder & operator=(const rebuilder &) = delete;
public:
  rebuilder();
  
  // Rebuilds sample and write it to a stream.
  static void rebuild(
    const sample & sample, 
    io::buffer_view sample_view,
    std::ostream & stream);
  
  // Rebuilds Vorbis headers and returns them as Ogg packets.
  static void rebuild_headers(
    int channels, int rate, std::uint32_t crc32,
    std::uint32_t loop_start, std::uint32_t loop_end,
    ogg_packet_holder & id,
    ogg_packet_holder & comment,
    ogg_packet_holder & setup);
  
  // Rebuilds identification header.
  static void rebuild_id_header(
    int channels, int rate, int blocksize_short, int blocksize_long, 
    ogg_packet_holder & packet);
  
  // Rebuilds comment header.
  static void rebuild_comment_header(
    ogg_packet_holder & packet,
    std::uint32_t loop_start, std::uint32_t loop_end);
  
  // Rebuilds setup header.
  static void rebuild_setup_header(
    const char * payload, std::size_t payload_size,
    ogg_packet_holder & packet);
};
  
}}

#endif

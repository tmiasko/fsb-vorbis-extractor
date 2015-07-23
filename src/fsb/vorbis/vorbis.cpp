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
#include "fsb/vorbis/vorbis.hpp"

#include <boost/crc.hpp>
#include <boost/utility/string_ref.hpp>
#include <glog/logging.h>

#include <algorithm>
#include <cstdlib>
#include <ostream>

namespace fsb { namespace vorbis {
  
std::uint32_t crc32(ogg_packet const &packet) {
  boost::crc_32_type result;
  result.process_bytes(packet.packet, packet.bytes);
  return {result.checksum()};
}
  
ogg_packet_holder::ogg_packet_holder()
: value {} {
}  
  
ogg_packet_holder::~ogg_packet_holder() {
  ogg_packet_clear(&value);
}

void ogg_packet_holder::assign(const unsigned char *buffer, std::size_t buffer_size) {
  unsigned char * const packet = 
    static_cast<unsigned char*>(std::malloc(buffer_size));
  CHECK(packet || !buffer_size) << "Failed to allocate memory.";
  std::copy_n(buffer, buffer_size, packet);
  std::free(value.packet);
  value.packet = packet;
  value.bytes = buffer_size;
}

void ogg_packet_holder::assign(const char *buffer, std::size_t buffer_size) {
  assign(reinterpret_cast<const unsigned char*>(buffer), buffer_size);
}

void ogg_packet_holder::clear() {
  ogg_packet_clear(&value);
}

vorbis_info_holder::vorbis_info_holder() {
  vorbis_info_init(&value);
}

vorbis_info_holder::~vorbis_info_holder() {
  vorbis_info_clear(&value);
}

vorbis_comment_holder::vorbis_comment_holder() {
  vorbis_comment_init(&value);
}

vorbis_comment_holder::~vorbis_comment_holder() {
  vorbis_comment_clear(&value);
}

ogg_ostream::ogg_ostream(int serial_number, std::ostream & output)
: output_(output) {
  CHECK(ogg_stream_init(&stream_state_, serial_number) == 0);
}
  
ogg_ostream::~ogg_ostream() {
  CHECK(ogg_stream_clear(&stream_state_) == 0);
}
  
void ogg_ostream::write_packet(ogg_packet & packet) {
  CHECK(ogg_stream_packetin(&stream_state_, &packet) == 0);
    
  ogg_page page;
  while (ogg_stream_pageout(&stream_state_, &page)) {
    write_page(page);
  }
}
  
void ogg_ostream::flush_packets() {
  ogg_page page;
  while (ogg_stream_flush(&stream_state_, &page)) {
    write_page(page);
  }
}
  
void ogg_ostream::write_page(const ogg_page & page) {
  CHECK(output_.write(reinterpret_cast<char*>(page.header), page.header_len));
  CHECK(output_.write(reinterpret_cast<char*>(page.body), page.body_len));
}

}}

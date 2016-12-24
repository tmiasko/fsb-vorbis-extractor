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
#ifndef FSB_VORBIS_VORBIS_H
#define FSB_VORBIS_VORBIS_H

#include "fsb/fsb.hpp"

#include <ogg/ogg.h>
#include <vorbis/codec.h>

#include <cstdint>
#include <iosfwd>

namespace fsb { namespace vorbis {

// Returns CRC-32 of Vorbis packet payload.
std::uint32_t crc32(ogg_packet const &packet);

// RAII holder for ogg_packet.
class ogg_packet_holder {
  ogg_packet_holder(const ogg_packet_holder &) = delete;
  ogg_packet_holder & operator=(const ogg_packet_holder &) = delete;
public:
  // Value initializes ogg_packet.
  ogg_packet_holder();
  // Clears packet with ogg_packet_clear.
  ~ogg_packet_holder();
  
  operator ogg_packet *() {
    return &value;
  }
  operator ogg_packet &() {
    return value;
  }
  ogg_packet * operator->() {
    return &value;
  }
  // Copies a content of the range [buffer, buffer + bytes) to a given Ogg 
  // packet. Updates packet and bytes fields.
  // 
  // Previous content of a packet is freed.
  void assign(const unsigned char *buffer, std::size_t buffer_size);
  void assign(const char *buffer, std::size_t buffer_size);
  
  // Clears packet content.
  void clear();
private:
  ogg_packet value;
};

// RAII wrapper for ogg_stream_state that forwards written data to std::ostream.
class ogg_ostream {
  ogg_ostream(const ogg_ostream &) = delete;
  ogg_ostream & operator=(const ogg_ostream &) = delete;
public:
  
  // Construct Ogg stream with given serial number, that will write it's output
  // to given output stream.
  ogg_ostream(int serial_number, std::ostream & output);
  
  // Destroys Ogg stream.
  ~ogg_ostream();
  
  // Submits packet to Ogg stream. Writes complete pages to output stream.
  void write_packet(ogg_packet & packet);
  
  // Flushes remaining packets inside the Ogg stream and forces them into pages.
  //
  // Note: It does not flush output stream.
  void flush_packets();
  
private:
  // Writes page header and page body to output stream.
  void write_page(const ogg_page & page);
  
private:
  std::ostream & output_;
  ogg_stream_state stream_state_;
};

// RAII holder for vorbis_info.
class vorbis_info_holder {
  vorbis_info_holder(const vorbis_info_holder &) = delete;
  vorbis_info_holder & operator=(const vorbis_info_holder &) = delete;
public:
  // Initializes vorbis_info with vorbis_info_init.
  vorbis_info_holder();
  // Clears vorbis_info with vorbis_info_clear.
  ~vorbis_info_holder();
  
  operator vorbis_info *() {
    return &value;
  }
  operator vorbis_info &() {
    return value;
  }
  vorbis_info * operator->() {
    return &value;
  }
private:
  vorbis_info value;
};

// RAII holder for vorbis_comment.
class vorbis_comment_holder {
  vorbis_comment_holder(const vorbis_comment_holder &) = delete;
  vorbis_comment_holder & operator=(const vorbis_comment_holder &) = delete;
public:
  // Intializes vorbis_comment with vorbis_comment_init,
  vorbis_comment_holder();
  // Add a tag-comment pair
  void add_tag(const char *tag, const char *contents);
  // Clears vorbis_comment with vorbis_comment_clear.
  ~vorbis_comment_holder();
  
  operator vorbis_comment *() {
    return &value;
  }
  operator vorbis_comment &() {
    return value;
  }
  vorbis_comment * operator->() {
    return &value;
  }
private:
  vorbis_comment value;
};

}}

#endif

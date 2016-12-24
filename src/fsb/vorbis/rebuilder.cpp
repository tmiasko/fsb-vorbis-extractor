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
#include "fsb/vorbis/rebuilder.hpp"

#include <boost/range/size.hpp>
#include <glog/logging.h>

namespace fsb { namespace vorbis {
  
rebuilder::rebuilder() {}

void rebuilder::rebuild(
  const sample & sample,
  io::buffer_view sample_view,
  std::ostream & stream) {
  
  vorbis_info_holder info;
  vorbis_comment_holder comment;
  
  long prev_blocksize = 0;
  ogg_int64_t prev_granulepos = 0;
  ogg_int64_t prev_packetno = 0;
  
  ogg_ostream ogg_stream(1, stream);
  
  {
    // Reconstruct and write Vorbis headers to stream.
    ogg_packet_holder header_id;
    ogg_packet_holder header_comment;
    ogg_packet_holder header_setup;

    rebuild_headers(
      sample.channels, sample.frequency, sample.vorbis_crc32,
      sample.loop_start, sample.loop_end,
      header_id, header_comment, header_setup);
    
    CHECK(vorbis_synthesis_headerin(info, comment, header_id) == 0);
    CHECK(vorbis_synthesis_headerin(info, comment, header_comment) == 0);
    CHECK(vorbis_synthesis_headerin(info, comment, header_setup) == 0);
    
    ogg_stream.write_packet(header_id);
    ogg_stream.write_packet(header_comment);
    ogg_stream.write_packet(header_setup);
    ogg_stream.flush_packets();
    
    prev_packetno = header_setup->packetno;
    prev_granulepos = 0;
  }

  {
    // Reconstruct audio packets.
    std::uint16_t packet_size = sample_view.read_uint16();
    while (packet_size) {
      ogg_packet packet {};
      packet.packet = 
        reinterpret_cast<unsigned char*>(
          const_cast<char*>(sample_view.read(packet_size)));
      packet.bytes = packet_size;
      packet.packetno = prev_packetno + 1;
      packet.granulepos = -1;
      
      // Read size of next packet to determine if we reached end of stream.
      packet_size = sample_view.offset() + 2 < sample_view.size() ?
        sample_view.read_uint16() : 0;
      packet.e_o_s = packet_size ? 0 : 1;
      
      // Update granulepos for packet.
      const long blocksize = vorbis_packet_blocksize(info, &packet);
      CHECK(blocksize > 0) << "vorbis_packet_blocksize failed: " << blocksize;
      packet.granulepos = prev_blocksize ?
        prev_granulepos + (blocksize + prev_blocksize) / 4 : 0;
      
      ogg_stream.write_packet(packet);
      
      prev_blocksize = blocksize;
      prev_granulepos = packet.granulepos;
      prev_packetno = packet.packetno;
    }
  }
}

namespace {
  
struct headers_info {
  std::uint32_t crc32;
  int blocksize_short;
  int blocksize_long;
  std::size_t setup_header_size;
  const char * setup_header;
};

struct headers_info_crc32_less {
  bool operator()(const headers_info & lhs, std::uint32_t rhs) const {
    return lhs.crc32 < rhs;
  }
};

// Vorbis setup headers map (ordered according to crc32).
const headers_info headers[] {
#include "fsb/vorbis/headers.inc"
};
const headers_info * const headers_end = headers + boost::size(headers);

}

void rebuilder::rebuild_headers(
  int channels, int rate, std::uint32_t crc32,
  std::uint32_t loop_start, std::uint32_t loop_end,
  ogg_packet_holder & id,
  ogg_packet_holder & comment,
  ogg_packet_holder & setup) {
  
  const auto i =
    std::lower_bound(headers, headers_end, crc32, headers_info_crc32_less());
  CHECK(i != headers_end && i->crc32 == crc32)
    << "Headers with CRC-32 equal " << crc32 << " not found.";
  
  rebuild_id_header(channels, rate, i->blocksize_short, i->blocksize_long, id);
  rebuild_comment_header(comment, loop_start, loop_end);
  rebuild_setup_header(i->setup_header, i->setup_header_size, setup);
}

namespace {

// Returns ceil(log2(x)) of non-negative integer.
int ilog2(int v) {
  CHECK(v > 0);

  int result = 0;
  if (v) {
    --v;
  }
  while (v) {
    result++;
    v >>= 1;
  }
  return result;
}

// Writes string to Ogg buffer as sequence of bytes.
void oggpack_write_string(oggpack_buffer *opb, boost::string_ref str) {
  for (auto c : str) {
    oggpack_write(opb, c, 8);
  }
}

}

void rebuilder::rebuild_id_header(
  int channels, int rate, int blocksize_short, int blocksize_long, 
  ogg_packet_holder & packet) {
  
  // Identification header
  oggpack_buffer opb;
  oggpack_writeinit(&opb);

  // Preamble
  oggpack_write(&opb, 0x01, 8);
  oggpack_write_string(&opb, "vorbis");

  // Basic information about the stream.
  oggpack_write(&opb, 0x00, 32);
  oggpack_write(&opb, channels, 8);
  oggpack_write(&opb, rate, 32);

  // Bitrate upper, nominal and lower.
  // All are optional and we do not provide them.
  oggpack_write(&opb, 0, 32);
  oggpack_write(&opb, 0, 32);
  oggpack_write(&opb, 0, 32);

  oggpack_write(&opb, ilog2(blocksize_short), 4);
  oggpack_write(&opb, ilog2(blocksize_long), 4);
  oggpack_write(&opb, 1, 1);

  CHECK(oggpack_writecheck(&opb) == 0);
  
  packet.assign(opb.buffer, oggpack_bytes(&opb));
  packet->b_o_s = 1;
  packet->e_o_s = 0;
  packet->granulepos = 0;
  packet->packetno = 0;
  
  oggpack_writeclear(&opb);
}

void rebuilder::rebuild_comment_header(
  ogg_packet_holder & packet,
  std::uint32_t loop_start, std::uint32_t loop_end) {
  // Comments header are generated using libvorbis for simplicity.
  vorbis_comment_holder comment;
  std::string s_loop_start;
  std::string s_loop_stop;
  if (loop_start != 0 && loop_end != 0) {
    std::string s_loop_start = std::to_string(loop_start);
    std::string s_loop_end = std::to_string(loop_end);
    comment.add_tag("LOOP_START", s_loop_start.c_str());
    comment.add_tag("LOOP_END", s_loop_end.c_str());
  }
  packet.clear();
  vorbis_commentheader_out(comment, packet);
}
  
void rebuilder::rebuild_setup_header(
  const char * payload, std::size_t payload_size,
  ogg_packet_holder & packet) {
  
  CHECK(payload_size != 0) << "Empty setup header.";
  
  // Setup header 
  packet.assign(payload, payload_size);
  packet->b_o_s = 0;
  packet->e_o_s = 0;
  packet->granulepos = 0;
  packet->packetno = 2;
}

}}
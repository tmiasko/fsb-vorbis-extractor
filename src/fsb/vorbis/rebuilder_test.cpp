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
#include "fsb/vorbis/headers_generator.hpp"
#include "fsb/vorbis/rebuilder.hpp"

#include <boost/range/size.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <vector>

using namespace fsb::vorbis;

namespace {

TEST(rebuilder_test, rebuild_id_header) {
  const int channels = 2;
  const int rate = 8000;
  
  ogg_packet_holder op;
  
  rebuilder::rebuild_id_header(
    channels, rate, 256, 512, op);

  ASSERT_EQ(30, op->bytes);
  
  // Preamble
  ASSERT_EQ(1, op->packet[0]);
  ASSERT_EQ('v', op->packet[1]);
  ASSERT_EQ('o', op->packet[2]);
  ASSERT_EQ('r', op->packet[3]);
  ASSERT_EQ('b', op->packet[4]);
  ASSERT_EQ('i', op->packet[5]);
  ASSERT_EQ('s', op->packet[6]);
  
  ASSERT_EQ(0, op->packet[7]);
  ASSERT_EQ(0, op->packet[8]);
  ASSERT_EQ(0, op->packet[9]);
  ASSERT_EQ(0, op->packet[10]);
  
  // Channels
  ASSERT_EQ(channels, op->packet[11]);
  
  // Rate
  ASSERT_EQ((rate >>  0) % 256, op->packet[12]);
  ASSERT_EQ((rate >>  8) % 256, op->packet[13]);
  ASSERT_EQ((rate >> 16) % 256, op->packet[14]);
  ASSERT_EQ((rate >> 24) % 256, op->packet[15]);
  
  // Bitrate upper
  ASSERT_EQ(0, op->packet[16]);
  ASSERT_EQ(0, op->packet[17]);
  ASSERT_EQ(0, op->packet[18]);
  ASSERT_EQ(0, op->packet[19]);
  
  // Bitrate nominal
  ASSERT_EQ(0, op->packet[20]);
  ASSERT_EQ(0, op->packet[21]);
  ASSERT_EQ(0, op->packet[22]);
  ASSERT_EQ(0, op->packet[23]);
  
  // Bitrate lower
  ASSERT_EQ(0, op->packet[24]);
  ASSERT_EQ(0, op->packet[25]);
  ASSERT_EQ(0, op->packet[26]);
  ASSERT_EQ(0, op->packet[27]);
  
  // Blocksizes
  ASSERT_EQ(8 | (9 << 4), op->packet[28]);
  
  // Framing
  ASSERT_EQ(1, op->packet[29]);
   
  ASSERT_EQ(1, op->b_o_s);
  ASSERT_EQ(0, op->e_o_s);
  ASSERT_EQ(0, op->granulepos);
  ASSERT_EQ(0, op->packetno);
}

TEST(rebuilder_test, rebuild_comment_header) {
  ogg_packet_holder op;
  
  rebuilder::rebuild_comment_header(op, 0, 20);
  
  ASSERT_EQ(0, op->b_o_s);
  ASSERT_EQ(0, op->e_o_s);
  ASSERT_EQ(0, op->granulepos);
  ASSERT_EQ(1, op->packetno);
}

TEST(rebuilder_test, rebuild_setup_header) {
  const char header[] { 1, 2, 3, 4};
  ogg_packet_holder op;
  
  rebuilder::rebuild_setup_header(header, boost::size(header), op);
  
  ASSERT_EQ(4, op->bytes);
  ASSERT_EQ(1, op->packet[0]);
  ASSERT_EQ(2, op->packet[1]);
  ASSERT_EQ(3, op->packet[2]);
  ASSERT_EQ(4, op->packet[3]);
  ASSERT_EQ(0, op->b_o_s);
  ASSERT_EQ(0, op->e_o_s);
  ASSERT_EQ(0, op->granulepos);
  ASSERT_EQ(2, op->packetno);
}

void assert_packets_eq(
  const ogg_packet &expected,
  const ogg_packet &actual) {
  
  ASSERT_EQ(expected.b_o_s, actual.b_o_s);
  ASSERT_EQ(expected.e_o_s, actual.e_o_s);
  ASSERT_EQ(expected.granulepos, actual.granulepos);
  ASSERT_EQ(expected.packetno, actual.packetno);
  
  ASSERT_EQ(expected.bytes, actual.bytes);
  
  for (long i=0; i < expected.bytes; ++i) {
    SCOPED_TRACE(i);
    ASSERT_EQ(expected.packet[i], actual.packet[i]);
  }
}

// Clears optional bitrate fields in Ogg identification header.
void clear_bitrate_fields(const ogg_packet & op) {
  std::fill_n(op.packet + 16, 12, 0);
}

class generate_and_rebuild_test
  : public testing::TestWithParam< std::tuple<int, int, int>> {
};


TEST_P(generate_and_rebuild_test, headers_are_equivalent) {
  const int channels = std::get<0>(GetParam());
  const int rate = std::get<1>(GetParam());
  const int quality = std::get<2>(GetParam());
  
  headers_generator generator(channels, rate, quality);
  
  ogg_packet_holder op_id_header;
  ogg_packet_holder op_comment_header;
  ogg_packet_holder op_setup_header;

  rebuilder::rebuild_headers(
    channels, rate, crc32(generator.setup_header()),
    0, 20,
    op_id_header, op_comment_header, op_setup_header);

  // Bitrate fields are not reconstructed, clear them before comparison.
  clear_bitrate_fields(generator.id_header());

  assert_packets_eq(generator.id_header(), op_id_header);
  assert_packets_eq(generator.comment_header(), op_comment_header);
  assert_packets_eq(generator.setup_header(), op_setup_header);
}

INSTANTIATE_TEST_CASE_P(
    all_combinations,
    generate_and_rebuild_test,
    testing::Combine(
      testing::Values(1, 2), 
      testing::Values(8000, 11000, 16000, 22050, 24000, 32000, 44100, 48000),
      testing::Range(1, 101)));
}

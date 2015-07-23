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

#include <gtest/gtest.h>

using namespace fsb::vorbis;

namespace {

TEST(crc32_test, correct_result_for_simple_data) {
  ogg_packet_holder packet;
  
  std::vector<unsigned char> payload;
  packet.assign(payload.data(), payload.size());
  ASSERT_EQ(0, crc32(packet));
  
  payload = {0xff, 0xff, 0xff, 0xff};
  packet.assign(payload.data(), payload.size());
  ASSERT_EQ(0xffffffff, crc32(packet));
  
  payload = {0x00, 0x00, 0x00, 0x00};
  packet.assign(payload.data(), payload.size());
  ASSERT_EQ(0x2144df1c, crc32(packet));
}

}
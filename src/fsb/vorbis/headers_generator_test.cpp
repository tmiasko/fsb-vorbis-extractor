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

#include <gtest/gtest.h>

using namespace fsb::vorbis;

namespace {
  
TEST(headers_generator_test, sanity_check) {
 headers_generator generator(2, 11000, 75);
 
 ASSERT_EQ(0, generator.id_header().packetno);
 ASSERT_TRUE(generator.id_header().bytes);
 ASSERT_EQ(1, generator.comment_header().packetno);
 ASSERT_TRUE(generator.comment_header().bytes);
 ASSERT_EQ(2, generator.setup_header().packetno);
 ASSERT_TRUE(generator.setup_header().bytes);
 
 ASSERT_EQ(2, generator.info().channels);
 ASSERT_EQ(11000, generator.info().rate);
}

}

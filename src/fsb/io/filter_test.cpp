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
#include "fsb/io/filter.hpp"

#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <gtest/gtest.h>

using namespace fsb::io;
namespace io = boost::iostreams;

namespace {

// Returns n-th bit of byte. Bits are indexed from 0 (lsb) to 7 (msb).
uint8_t get_nth_bit(uint8_t x, uint8_t n) {
  return (x >> n) & 1u;
}

class reverse_bits_test : public testing::TestWithParam<int> {};
  
TEST_P(reverse_bits_test, bits_are_reversed) {
  const uint8_t x = GetParam();
  const uint8_t y = reverse_bits(x);
  ASSERT_EQ(get_nth_bit(x, 7), get_nth_bit(y, 0));
  ASSERT_EQ(get_nth_bit(x, 6), get_nth_bit(y, 1));
  ASSERT_EQ(get_nth_bit(x, 5), get_nth_bit(y, 2));
  ASSERT_EQ(get_nth_bit(x, 4), get_nth_bit(y, 3));
  ASSERT_EQ(get_nth_bit(x, 3), get_nth_bit(y, 4));
  ASSERT_EQ(get_nth_bit(x, 2), get_nth_bit(y, 5));
  ASSERT_EQ(get_nth_bit(x, 1), get_nth_bit(y, 6));
  ASSERT_EQ(get_nth_bit(x, 0), get_nth_bit(y, 7));
}

INSTANTIATE_TEST_CASE_P(
  all_bytes,
  reverse_bits_test,
  testing::Range(0, 256));

TEST(reverse_bits_filter_test, bits_are_reversed) {
  const std::string input {0b01110000u, 0b00101010u};
  std::string output;

  io::filtering_ostream out;
  out.push(reverse_bits_filter());
  out.push(io::back_inserter(output));
  out << input;
  out.flush();
  
  ASSERT_EQ(2u, output.size());
  ASSERT_EQ(0b00001110u, output[0]);
  ASSERT_EQ(0b01010100u, output[1]);
}

TEST(xor_filter_test, filtering_twice_is_identity) {
  const std::string key { "secret key!" };
  const std::string input { "message text, foo bar, baz, ...." };
  
  std::string xored_once;
  {
    io::filtering_ostream out;
    out.push(xor_filter(key));
    out.push(io::back_inserter(xored_once));
    out << input;
    out.flush();
  }
  
  ASSERT_EQ(input.size(), xored_once.size());
  ASSERT_NE(input, xored_once);
  
  std::string xored_twice;
  {
    io::filtering_ostream out;
    out.push(xor_filter(key));
    out.push(io::back_inserter(xored_twice));
    out << xored_once;
    out.flush();
  }
  
  ASSERT_NE(xored_once, xored_twice);
  ASSERT_EQ(input, xored_twice);
}

}

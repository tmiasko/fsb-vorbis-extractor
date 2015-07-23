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
#ifndef FSB_IO_FILTER_H
#define FSB_IO_FILTER_H

#include <boost/iostreams/filter/symmetric.hpp>
#include <boost/utility/string_ref.hpp>

#include <cstdint>

namespace fsb { namespace io {

// Reverses order of bits in a byte.
inline std::uint8_t reverse_bits(std::uint8_t x) {
  x = (x & 0b11110000u) >> 4 | (x & 0b00001111) << 4;
  x = (x & 0b11001100u) >> 2 | (x & 0b00110011) << 2;
  x = (x & 0b10101010u) >> 1 | (x & 0b01010101) << 1;
  return x;
}

// Filter reversing order of bits in a stream.
//
// Model of SymmetricFilter.nt8_t
class reverse_bits_filter_impl {
public:
  // Type of processed characters.
  typedef char char_type;
  
  bool filter(
    const char * &src_begin, const char * src_end,
    char * &dst_begin, char * dst_end,
    bool flush);
  
  void close();
};

struct reverse_bits_filter
  : boost::iostreams::symmetric_filter<reverse_bits_filter_impl> {
  reverse_bits_filter();
};

// Filter xor-ing input with a given key.
//
// Model of SymmetricFilter.
class xor_filter_impl {
public:
  // Type of processed characters.
  typedef char char_type;
  
  // Constructs filter with a given non-empty key.
  xor_filter_impl(boost::string_ref key);
  
  bool filter(
    const char * &src_begin, const char * src_end,
    char * &dst_begin, char * dst_end,
    bool flush);
  
  void close();
private:
  const std::string key_;
  const char * key_position_;
};

struct xor_filter
  : boost::iostreams::symmetric_filter<xor_filter_impl> {
  // Constructs filter with a given non-empty key.
  xor_filter(boost::string_ref key);
};

}}

#endif

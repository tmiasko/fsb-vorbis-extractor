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

#include <glog/logging.h>

namespace fsb { namespace io {

bool reverse_bits_filter_impl::filter(
  const char * &src_begin, const char * src_end,
  char * &dst_begin, char * dst_end,
  bool /* flush */) {
    
  while (src_begin != src_end && dst_begin != dst_end) {
    *reinterpret_cast<std::uint8_t*>(dst_begin++) = 
      reverse_bits(*reinterpret_cast<const std::uint8_t*>(src_begin++));
  }
    
  return false;
}
  
void reverse_bits_filter_impl::close() {
}

reverse_bits_filter::reverse_bits_filter()
  : boost::iostreams::symmetric_filter<reverse_bits_filter_impl>(1024) {
}

xor_filter_impl::xor_filter_impl(boost::string_ref key)
  : key_(key)
  , key_position_(key_.data()) {
  CHECK(!key_.empty());
}

bool xor_filter_impl::filter(
  const char * &src_begin, const char * src_end,
  char * &dst_begin, char * dst_end,
  bool /* flush */) {

  const char * key_begin = key_.data();
  const char * key_end = key_.data() + key_.size();

  while (src_begin != src_end && dst_begin != dst_end) {
    *dst_begin++ = *key_position_++ ^ *src_begin++;
    if (key_position_ == key_end) {
      key_position_ = key_begin;
    }
  }

  return false;
}

void xor_filter_impl::close() {
  key_position_ = key_.data();
}

xor_filter::xor_filter(boost::string_ref key)
  : boost::iostreams::symmetric_filter<xor_filter_impl>(1024, key) {
}

}}

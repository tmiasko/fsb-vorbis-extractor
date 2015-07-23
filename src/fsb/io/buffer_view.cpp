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
#include "fsb/io/buffer_view.hpp"

#include <glog/logging.h>

namespace fsb { namespace io {
  
buffer_view::buffer_view(const char * buffer, std::size_t length)
: begin_(buffer)
, end_(buffer + length)
, current_(buffer)
{}

buffer_view::buffer_view(const char * begin, const char * end)
: begin_(begin)
, end_(end)
, current_(begin)
{}

void buffer_view::set_offset(std::size_t offset) {
  CHECK(offset <= size());
  current_ = begin_ + offset;
}

void buffer_view::skip(std::size_t length) {
  CHECK(remaining() >= length);
  current_ += length;
}

const char * buffer_view::read(std::size_t length) {
  CHECK(remaining() >= length);
  const char * const result = current_;
  current_ += length;
  return result;
}

char buffer_view::read_char() {
  CHECK(remaining() >= 1u);
  return *current_++;
}

uint8_t buffer_view::read_uint8() {
  CHECK(remaining() >= 1u);
  return *current_++;
}

uint16_t buffer_view::read_uint16() {
CHECK(remaining() >= 2u);
  const uint16_t x = read_uint8();
  const uint16_t y = read_uint8();
  return x + (y << 8u);
}

uint32_t buffer_view::read_uint24() {
  CHECK(remaining() >= 3u);
  const uint32_t x = read_uint8();
  const uint32_t y = read_uint16();
  return x + (y << 8u);
}

uint32_t buffer_view::read_uint32() {
  CHECK(remaining() >= 4u);
  const uint32_t x = read_uint16();
  const uint32_t y = read_uint16();
  return x + (y << 16u);
}

uint64_t buffer_view::read_uint64() {
  CHECK(remaining() >= 8u);
  const uint64_t x = read_uint32();
  const uint64_t y = read_uint32();
  return x + (y << 32u);
}
  
}}

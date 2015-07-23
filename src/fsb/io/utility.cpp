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
#include "fsb/io/utility.hpp"

#include <glog/logging.h>

#include <istream>
#include <limits>

namespace fsb { namespace io {

void read(std::istream & stream, char * buffer, std::size_t size) {
  const std::streamsize streamsize_max =
    std::numeric_limits<std::streamsize>::max();
  while (size > 0) {
    const std::streamsize to_read = streamsize_max < size ?
      streamsize_max : size;
    CHECK(stream.read(buffer, to_read));
    size -= to_read;
  }
}

std::vector<char> read(std::istream & stream, std::size_t size) {
  std::vector<char> buffer(size);
  read(stream, buffer.data(), buffer.size());
  return buffer;
}

}}

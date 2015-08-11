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
#ifndef FSB_IO_BUFFER_VIEW_HPP
#define FSB_IO_BUFFER_VIEW_HPP

#include <cstddef>
#include <cstdint>

namespace fsb { namespace io {

// View on sequence of bytes that allows to read them as integers of different 
// sizes. Holds and offset within buffer that is updated by a number of bytes
// read, after each read operation.
//
// Does not retain ownership of given byte buffer.
class buffer_view {
public:
  buffer_view(const char * buffer, std::size_t length);
  buffer_view(const char * begin, const char * end);
  
  // Returns pointer to the first character of buffer.
  const char * begin() const {
    return begin_;
  }
  
  // Returns a pointer to the past-the-end character of buffer.
  const char * end() const {
    return end_;
  }
  
  // Returns a pointer to the current character of buffer.
  const char * current() const {
    return current_;
  }

  // Returns true if the buffer is empty.
  bool empty() const {
    return end_ == begin_;
  }
  
  // Returns the size of the buffer.
  std::size_t size() const {
    return end_ - begin_;
  }

  // Returns number of bytes remaining in the buffer from current offset.
  std::size_t remaining() const {
    return end_ - current_;
  }
  
  // Returns current position in the buffer.
  std::size_t offset() const {
    return current_ - begin_;
  }
  
  // Next read will start from given offset.
  void     set_offset(std::size_t offset);
  // Skips next length bytes.
  void     skip(std::size_t length);
  // Reads length bytes and returns pointer to the beginning of read data.
  const char * read(std::size_t length);
  // Reads character.
  char     read_char();
  // Reads 8-bit unsigned integer.
  std::uint8_t  read_uint8();
  // Reads 16-bit unsigned integer.
  std::uint16_t read_uint16();
  // Reads 24-bit unsigned integer.
  std::uint32_t read_uint24();
  // Reads 32-bit unsigned integer.
  std::uint32_t read_uint32();
  // Reads 64-bit unsigned integer.
  std::uint64_t read_uint64();
  
private:
  const char * begin_;
  const char * end_;
  const char * current_;
};

}}

#endif

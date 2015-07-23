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
#ifndef FSB_FSB_HPP
#define FSB_FSB_HPP

#include <boost/utility/string_ref.hpp>

#include <cstdint>
#include <istream>
#include <string>

namespace fsb {

// Sample format
enum class format : std::uint32_t {
  none,     // Unitialized / unknown.
  pcm8,     // 8bit integer PCM data.
  pcm16,    // 16bit integer PCM data.
  pcm24,    // 24bit integer PCM data.
  pcm32,    // 32bit integer PCM data.
  pcmfloat, // 32bit floating point PCM data.
  gcadpcm,  // Compressed Nintendo 3DS/Wii DSP data.
  imaadpcm, // Compressed IMA ADPCM data.
  vag,      // Compressed PlayStation Portable ADPCM data.
  hevag,    // Compressed PSVita ADPCM data.
  xma,      // Compressed Xbox360 XMA data.
  mpeg,     // Compressed MPEG layer 2 or 3 data.
  celt,     // Compressed CELT data.
  at9,      // Compressed PSVita ATRAC9 data.
  xwma,     // Compressed Xbox360 xWMA data.
  vorbis,   // Compressed Vorbis data.
  max,      // Maximum number of sound formats supported.
};

// FSB file header.
struct header {
  // FSB file format identifier.
  char   id[4];
  // File format version.
  std::uint32_t version = 0;
  // Number of samples in a file.
  std::uint32_t samples = 0;
  // Size of section with sample headers.
  std::uint32_t headers_size = 0;
  // Size of section with sample names.
  std::uint32_t names_size = 0;
  // Size of section with sample data.
  std::uint32_t data_size = 0;
  // Samples format.
  format mode;
  // ?
  std::uint64_t unknown;
  // Some kind of hash.
  std::int8_t guid[24];
};

// FSB sample.
struct sample {
  // Sample name.
  std::string name;
  // Sample rate.
  std::uint32_t frequency = 0;
  // Number of channels in sample.
  std::uint8_t channels = 0;
  // Position of sample expressed as an offset from start of data section.
  std::size_t offset = 0;
  // Size of sample in data section.
  std::size_t size = 0;
  // CRC-32 of Vorbis setup header.
  std::uint32_t vorbis_crc32 = 0;
  std::uint32_t loop_start = 0;
  std::uint32_t loop_end = 0;
  std::uint32_t unknown = 0;
};

}

#endif
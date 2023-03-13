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
#include "fsb/container.hpp"

#include "fsb/fsb.hpp"
#include "fsb/io/filter.hpp"
#include "fsb/io/utility.hpp"
#include "fsb/vorbis/rebuilder.hpp"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/utility/string_ref.hpp>
#include <glog/logging.h>

namespace fsb {
 
container::container(std::istream & encoded_stream, boost::string_ref password) {
  boost::iostreams::filtering_istream stream;
  if (!password.empty()) {
    // Decrypt stream if password is provided.
    stream.push(fsb::io::xor_filter(password));
    stream.push(fsb::io::reverse_bits_filter());
  }
  stream.push(encoded_stream);
  
  read_file_header(stream);
  read_sample_headers(stream);
  read_sample_names(stream);
  data_buffer_ = io::read(stream, header_.data_size);
}

void container::read_file_header(std::istream & stream) {
  const std::vector<char> header_buffer = io::read(stream, header_size);
  io::buffer_view header_view(header_buffer.data(), header_buffer.size());
  
  header_.id[0] = header_view.read_char();
  header_.id[1] = header_view.read_char();
  header_.id[2] = header_view.read_char();
  header_.id[3] = header_view.read_char();

  CHECK(std::equal(header_.id, header_.id + 4, "FSB5"));

  header_.version = header_view.read_uint32();

  CHECK(header_.version == 1);

  header_.samples  = header_view.read_uint32();
  header_.headers_size = header_view.read_uint32();
  header_.names_size = header_view.read_uint32();
  header_.data_size = header_view.read_uint32();
  
  const std::uint32_t mode = header_view.read_uint32();
  CHECK(0 < mode && mode < static_cast<std::uint32_t>(format::max))
    << "Mode field is out of range: " << mode;
  header_.mode = static_cast<format>(mode);
  
  header_.unknown = header_view.read_uint64();

  for (int i=0; i < 24; ++i)
    header_.guid[i] = header_view.read_uint8();
}

void container::read_sample_headers(std::istream & stream) {
  const std::vector<char> samples_buffer = io::read(stream, header_.headers_size);
  io::buffer_view samples_view(samples_buffer.data(), samples_buffer.size());
  
  for (std::uint32_t i=0; i < header_.samples; ++i) {
    samples_.push_back(read_sample_header(samples_view));
  }
    
  // Calculate sample sizes, assuming that samples are laid out contiguously.
  const sample * next_sample {};
  for (auto & sample : boost::adaptors::reverse(samples_)) {
    const auto next_offset = next_sample ?
      next_sample->offset : header_.data_size;
    CHECK(sample.offset < next_offset);
    sample.size = next_offset - sample.offset;
    next_sample = &sample;
  }  
}

sample container::read_sample_header(io::buffer_view & view) {
  sample sample;
  
  // Bits:
  // 0       has more flags
  // 1 2 3 4 frequency
  // 5 6     log2(channels)
  // 7       first bit of sample offset
  const std::uint8_t mode = view.read_uint8();
  bool has_extra_headers = mode & 1u;
  sample.channels = 1u << ((mode & 96u) >> 5u);

  switch ((mode & 31u) >> 1u) {
    case 1u: sample.frequency =  8000u; break;
    case 2u: sample.frequency = 11000u; break;
    case 3u: sample.frequency = 11025u; break;
    case 4u: sample.frequency = 16000u; break;
    case 5u: sample.frequency = 22050u; break;
    case 6u: sample.frequency = 24000u; break;
    case 7u: sample.frequency = 32000u; break;
    case 8u: sample.frequency = 44100u; break;
    case 9u: sample.frequency = 48000u; break;
    default: CHECK(false); break;
  }
  
  const std::size_t sample_offset_bit_0      = mode >> 7u;
  const std::size_t sample_offset_bit_1_plus = view.read_uint24();
  // Samples have 32 byte alignment.
  sample.offset = (sample_offset_bit_0 | sample_offset_bit_1_plus << 1u) << 5u;
  // ???
  sample.unknown = view.read_uint32();

  while (has_extra_headers) {
    // Bits:
    // 0        has extra headers
    // 1 ... 23 length
    const uint32_t extra = view.read_uint24();
    has_extra_headers = extra & 0x01;
    std::size_t extra_length = extra >> 1u;
    const uint8_t type = view.read_uint8();

    switch (type) {
      case 0x02: 
        CHECK(extra_length == 1);
        sample.channels = view.read_uint8(); 
        break;
      case 0x04: 
        CHECK(extra_length == 4);
        sample.frequency = view.read_uint32(); 
        break;
      case 0x06: 
        CHECK(extra_length == 8);
        sample.loop_start = view.read_uint32();
        sample.loop_end = view.read_uint32();
        break;
      case 0x16:
        CHECK(extra_length >= 4);
        sample.vorbis_crc32 = view.read_uint32();
        extra_length -= 4;
        // Maybe position / seek information (granulepos)?
        CHECK(extra_length % 4 == 0);
        // Seek information. Two sequences non-decreasing sequences of 32 bit
        // numbers laid out as follows:
        // a_1, b_1, a_2, b_2, ..., a_n-1, b_n-1, a_n
        // Where a_i are offsets within sample audio data (pointing at beginning
        // of packets), and b_i are associated granulepos.
        // But there are granulepos only for some packets, not for all of them.
        view.skip(extra_length);
        break;
      default:
        //CHECK(false) << "Unexpected extra header type: " << int(type) << ", length: " << extra_length;
        view.skip(extra_length);
    }
  }
  
  return sample;
}

void container::read_sample_names(std::istream & stream) {
  const std::vector<char> names_buffer = io::read(stream, header_.names_size);
  io::buffer_view names_view(names_buffer.data(), names_buffer.size());

  for (std::uint32_t i=0; i < header_.samples; ++i) {
    if (names_view.empty()) {
      samples_[i].name = "sample";
    } else {
      names_view.set_offset(i * 4u);
      names_view.set_offset(names_view.read_uint32());
      while (char c = names_view.read_char()) {
        samples_[i].name.push_back(c);
      }
    }
  }
}


void container::extract_sample(const sample & sample, std::ostream & stream) {
  CHECK(header_.mode == format::vorbis);
  
  // Construct sample data view to verify that we don't exceed sample boundaries 
  // during extraction process.
  CHECK(sample.offset <= data_buffer_.size());
  CHECK(sample.offset + sample.size <= data_buffer_.size());
  const char * const sample_begin = data_buffer_.data() + sample.offset;
  const char * const sample_end = sample_begin + sample.size;
  io::buffer_view sample_view(sample_begin, sample_end);
  vorbis::rebuilder rebuilder;
  rebuilder.rebuild(sample, sample_view, stream);
}

}

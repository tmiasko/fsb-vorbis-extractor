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

#include <boost/filesystem.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>

DEFINE_string(password, "", 
  "password used to encode file");
DEFINE_string(destination, "",
  "directory where extracted files will be placed, "
  "current working directory is used by default");
DEFINE_bool(list, false, 
  "only list content of container without extracting content");

namespace {

struct extractor_options {
  bool extract;
  std::string password;
  boost::filesystem::path destination;
  std::vector<boost::filesystem::path> paths;
};

extractor_options parse_options(int argc, char **argv) {
  extractor_options options;
  options.password = FLAGS_password;
  options.destination = FLAGS_destination.empty() ?
    boost::filesystem::current_path() : FLAGS_destination;
  options.extract = !FLAGS_list;
  
  for (int i=1; i < argc; ++i) {
    options.paths.push_back(argv[i]);
  }
  
  return options;
}

void print_header(std::ostream & os, const fsb::header & header) {
  os
    << "Version:       " << header.version << '\n'
    << "Samples:       " << header.samples << '\n'
    << "Headers size:  " << header.headers_size << '\n'
    << "Names size:    " << header.names_size << '\n'
    << "Data size:     " << header.data_size << '\n'
    << "Format:        " << int(header.mode) << '\n'
    << "Unknown:       " << header.unknown << '\n';
}

void print_sample(std::ostream & os, const fsb::sample & sample) {
  os
    << "Name:          " << sample.name << '\n'
    << "Frequency:     " << sample.frequency << '\n'
    << "Channels:      " << int(sample.channels) << '\n'
    << "Offset:        " << sample.offset << '\n'
    << "Size:          " << sample.size << '\n'
    << "Vorbis CRC-32: " << sample.vorbis_crc32 << '\n'
    << "Loop start:    " << sample.loop_start << '\n'
    << "Loop end:      " << sample.loop_end << '\n'
    << "Unknown:       " << sample.unknown << '\n';
}

}

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  google::SetUsageMessage(std::string(argv[0]) + " file.fsb");
  google::ParseCommandLineFlags(&argc, &argv, true);

  std::size_t sample_number = 0;
  const extractor_options options = parse_options(argc, argv);
  
  for (const auto & path : options.paths) {
    std::ifstream stream(path.native(),
      std::ios_base::in | std::ios_base::binary);
    CHECK(stream) << "Failed to open path: " << path.native();
    fsb::container container(stream, options.password);
    
    auto & header = container.file_header();
    std::cout << path.native() << std::endl;
    print_header(std::cout, header);
    std::cout << std::endl;
    
    for (auto & sample : container.samples()) {
      sample_number += 1;
      
      print_sample(std::cout, sample);
      std::cout << std::endl;
      
      if (options.extract) {
        const boost::filesystem::path path = options.destination / 
          (std::to_string(sample_number) + "." + sample.name + ".ogg");
        if (boost::filesystem::exists(path)) {
          std::cerr 
            << "Destination already exists, skipping: " 
            << sample.name << std::endl;
          continue;
        }
        std::ofstream output(path.native());
        container.extract_sample(sample, output);
      }
    }
  }
  
  return 0;
}
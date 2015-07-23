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

#include <gtest/gtest.h>

#include <sstream>

namespace {

TEST(read_test, succesfull_read) {
  std::istringstream in { "12345678" };
  char buffer[4];
  fsb::io::read(in, buffer, 4);
  
  ASSERT_EQ("1234", std::string(buffer, buffer + 4));
}

TEST(read_test, unsuccesfull_read) {
  std::istringstream in { "12" };
  char buffer[4];
  
  ASSERT_DEATH(fsb::io::read(in, buffer, 4), "");
}

}
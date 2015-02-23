// Copyright (C) 2015 team-diana MIT license

#include "test_utils.hpp"

std::string generateString(unsigned int size) {
  std::string s;
  s.reserve(size);
  for(unsigned int i = 0; i < size; i++) {
    s+='a'+i%25;
  }
  return s;
}


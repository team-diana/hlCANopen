// Copyright (C) 2015 team-diana MIT license

#pragma once

#include <string>
#include <type_traits>
#include <sstream>

#include <folly/futures/Future.h>

#include "hlcanopen/sdo_error.hpp"


std::string generateString(unsigned int size);

template <typename T> hlcanopen::SdoError getSdoError(const folly::Future<T>& e) {
    try {
        e.value();
    } catch (const hlcanopen::SdoError& e) {
        return e;
    }
    throw std::runtime_error("this future has not a SdoError");
}

template <typename C, class = typename std::enable_if<!std::is_array<C>::value>::type, class = int >
  std::string iterableToString(const C& iterable, std::string separator = " ") {
      std::stringstream ss;

      typename C::const_iterator begin = iterable.begin();
      typename C::const_iterator end = iterable.end();

      for (typename C::const_iterator i = begin; i != end; ++i) {
          if(std::distance(i, end) > 1) {
            ss << *i << separator;
          } else {
            ss << *i;
          }
      }

      return ss.str();
}

void registLoggers();

struct TestFixture {

    TestFixture();
    ~TestFixture();

};

// Copyright (C) 2015 team-diana MIT license

#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <string>

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

void registLoggers();

struct TestFixture {

    TestFixture();
    ~TestFixture();

};

#endif // TEST_UTILS_HPP


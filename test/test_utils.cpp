// Copyright (C) 2015 team-diana MIT license

#include "test_utils.hpp"

#include <hlcanopen/logging/easylogging++.h>

#include <vector>

std::string generateString(unsigned int size) {
    std::string s;
    s.reserve(size);
    for(unsigned int i = 0; i < size; i++) {
        s+='a'+i%25;
    }
    return s;
}

void registLoggers() {
    std::vector<const char*> customLoggerNames {"default", "interface", "canopen_manager", "sdo", "pdo"};
    for(auto name: customLoggerNames) {
        el::Loggers::getLogger(name);
    }
}


TestFixture::TestFixture()
{
    registLoggers();
}

TestFixture::~TestFixture()
{

}

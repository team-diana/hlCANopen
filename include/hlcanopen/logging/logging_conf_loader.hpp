// Copyright (C) 2015 team-diana MIT license

#ifndef LOGGING_CONF_LOADER_HPP
#define LOGGING_CONF_LOADER_HPP

#include <boost/filesystem.hpp>

#include "hlcanopen/logging/easylogging++.h"

#include <iostream>

namespace hlcanopen {

void initCustomLogger(const char* loggerName, const boost::filesystem::path confFilePath);

void loadDefaultConfig(const boost::filesystem::path& confDirAbs);

void loadConfigFilesFromDir(const char* confDirPath);

}

#endif // LOGGING_CONF_LOADER_HPP

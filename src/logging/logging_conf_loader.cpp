// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/logging/logging_conf_loader.hpp"

namespace hlcanopen {

void initCustomLogger(const char* loggerName, const boost::filesystem::path confFilePath) {
    el::Loggers::getLogger(loggerName);


    if(boost::filesystem::exists(confFilePath)) {
//         std::cout << "Reading from : " << confFilePath << std::endl;
        el::Configurations conf(confFilePath.c_str());
        el::Loggers::reconfigureLogger(loggerName, conf);
    }
}

void loadDefaultConfig(const boost::filesystem::path& confDirAbs) {
    boost::filesystem::path confFileAbs = confDirAbs;
    confFileAbs.append(std::string("./") + "default.conf");

    if(boost::filesystem::exists(confFileAbs)) {
//         std::cout << "Reading from : " << confFileAbs << std::endl;
        el::Configurations conf(confFileAbs.c_str());
        el::Loggers::reconfigureLogger("default", conf);
        el::Loggers::setDefaultConfigurations(conf);
    }
}


void loadConfigFilesFromDir(const char* confDirPath) {
    boost::filesystem::path confDir(confDirPath);
    boost::filesystem::path confDirAbs = boost::filesystem::absolute(confDir);

    if(!boost::filesystem::exists(confDirAbs)) {
        std::cerr << "LOGGING ERROR: cannot find configuration directory: '"
                  << confDirAbs << "'";
    } else {
        loadDefaultConfig(confDirAbs);

        std::vector<const char*> customLoggerNames {"interface", "canopen_manager", "sdo", "pdo"};
        for(auto name : customLoggerNames) {
            boost::filesystem::path confFile = confDirAbs;
            confFile.append(std::string("./") + name + ".conf");
            initCustomLogger(name, confFile);
        };
    }

}

}

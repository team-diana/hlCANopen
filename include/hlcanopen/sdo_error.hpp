// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_ERROR_HPP
#define OCO_SDO_ERROR_HPP

#include <string>

namespace hlcanopen {

enum SdoErrorCode {
    NO_ERROR = 0x0,
    TOGGLE_BIT_NOT_ALTERNATED = 0x05030000,
    INVALID_UNKNOWN_COMMAND_SPEC = 0x05040001,
    OUT_OF_MEMORY = 0x05040005,
    UNSUPPORTED_ACCESS = 0x06010000,
    READ_WRITEONLY = 0x06010001,
    WRITE_READONLY = 0x06010002,
    OBJECT_NOT_IN_DICT = 0x06020000,
    TIMEOUT = 0xff,
    // TODO add all other errors
};

class SdoError : public std::exception {

public:
    explicit SdoError();
    SdoError(SdoErrorCode errorCode);

    virtual const char* what() const noexcept override;

    bool isNoError();

private:
    SdoErrorCode errorCode;
};

}

#endif // OCO_SDO_ERROR_HPP


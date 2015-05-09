// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/sdo_error.hpp"
#include "hlcanopen/utils.hpp"

namespace hlcanopen {
SdoError::SdoError(SdoErrorCode errorCode) :
errorCode(errorCode)
{

}

std::string SdoError::string()
{
  switch(errorCode) {
    case NO_ERROR: return "NO_ERROR"; break;
    case TOGGLE_BIT_NOT_ALTERNATED: return "TOGGLE_BIT_NOT_ALTERNATED"; break;
    case INVALID_UNKNOWN_COMMAND_SPEC : return "INVALID_UNKNOWN_COMMAND_SPEC "; break;
    case OUT_OF_MEMORY : return "OUT_OF_MEMORY "; break;
    case UNSUPPORTED_ACCESS : return "UNSUPPORTED_ACCESS "; break;
    case READ_WRITEONLY : return "READ_WRITEONLY "; break;
    case WRITE_READONLY : return "WRITE_READONLY "; break;
    case OBJECT_NOT_IN_DICT : return "OBJECT_NOT_IN_DICT "; break;
    case TIMEOUT : return "TIMEOUT "; break;
    default: return "";
  }
}

bool SdoError::is_no_error()
{
  return errorCode == SdoErrorCode::NO_ERROR;
}


}


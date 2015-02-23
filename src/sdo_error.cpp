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
  NOT_IMPLEMENTED_YET;
  return ""; // Lookup in static array;
}

bool SdoError::is_no_error()
{
  return errorCode == SdoErrorCode::NO_ERROR;
}


}


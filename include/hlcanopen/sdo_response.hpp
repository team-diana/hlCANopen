// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_RESPONSE
#define OCO_SDO_RESPONSE

#include "hlcanopen/sdo_error.hpp"

#include <future>

namespace hlcanopen {

template<typename T> class SdoResponse {
public:
  SdoResponse(T result, SdoError error = SdoError()) :
  result(result),
  sdoError(error)
  {}

  SdoResponse(SdoError error = SdoError()) :
  sdoError(error)
  {}

  T get() {
    return result;
  }

  bool ok() {
    return sdoError.is_no_error();
  }

  SdoError getError() {
    return sdoError;
  }

  void setError(SdoError error) {
    sdoError = error;
  }

private:
  T result;
  SdoError sdoError;
};

}

#endif // OCO_SDO_RESPONSE


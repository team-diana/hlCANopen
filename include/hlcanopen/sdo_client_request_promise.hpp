// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_REQUEST_PROMISE_HPP
#define OCO_SDO_CLIENT_REQUEST_PROMISE_HPP

// #include <boost/variant.hpp>

#include "hlcanopen/sdo_client_request.hpp"
#include "hlcanopen/sdo_response.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include <thread>
#include <future>
#include <cstdint>
#include <functional>

namespace hlcanopen {

template<typename T> class SdoClientReadRequestPromise : public SdoClientReadRequest {
public:
  SdoClientReadRequestPromise(SDOIndex sdoIndex, long timeout) :
   SdoClientReadRequest(sdoIndex, timeout),
   promise() {

  }
  virtual ~SdoClientReadRequestPromise() {}

  void completeRequest(const SdoData& data) override {
    promise.set_value(SdoResponse<T>(convertSdoData<T>(data)));
  }

  void completeRequestWithFail(const SdoError& error) override {
    promise.set_value(SdoResponse<T>(error));
  }

  void completeRequestWithTimeout() override { /* XXX */
    const SdoError error(SdoErrorCode::TIMEOUT);
    promise.set_value(SdoResponse<T>(error));
  }

  std::future<SdoResponse<T>> getFuture() {
    return promise.get_future();
  }

private:
  std::promise<SdoResponse<T>> promise;
};

class SdoClientWriteRequestPromise : public SdoClientWriteRequest {
public:
  SdoClientWriteRequestPromise(SDOIndex sdoIndex, SdoData sdoData, long timeout) :
   SdoClientWriteRequest(sdoIndex, sdoData, timeout),
   promise() {}
  virtual ~SdoClientWriteRequestPromise() {}

  void completeRequest() override {
    promise.set_value(SdoResponse<bool>(true));
  }

  void completeRequestWithFail(const SdoError& error) override {
    promise.set_value(SdoResponse<bool>(true, error));
  }

  void completeRequestWithTimeout() override {
    const SdoError error = SdoError(SdoErrorCode::TIMEOUT);
    promise.set_value(SdoResponse<bool>(false, error)); /* XXX */
  }

  std::future<SdoResponse<bool>> getFuture() {
    return promise.get_future();
  }

private:
  std::promise<SdoResponse<bool>> promise;
};

}

#endif // OCO_SDO_CLIENT_REQUEST_PROMISE_HPP


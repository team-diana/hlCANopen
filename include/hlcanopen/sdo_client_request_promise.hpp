// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_REQUEST_PROMISE_HPP
#define OCO_SDO_CLIENT_REQUEST_PROMISE_HPP

// #include <boost/variant.hpp>

#include "hlcanopen/sdo_client_request.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include <folly/futures/Future.h>

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
    promise.setValue(convertSdoData<T>(data));
  }

  void completeRequestWithFail(const SdoError& error) override {
    promise.setException(error);
  }

  void completeRequestWithTimeout() override {
    const SdoError error(SdoErrorCode::TIMEOUT);
    promise.setException(error);
  }

  folly::Future<T> getFuture() {
    return promise.getFuture();
  }

private:
  folly::Promise<T> promise;
};

class SdoClientWriteRequestPromise : public SdoClientWriteRequest {
public:
  SdoClientWriteRequestPromise(SDOIndex sdoIndex, SdoData sdoData, long timeout) :
   SdoClientWriteRequest(sdoIndex, sdoData, timeout),
   promise() {}
  virtual ~SdoClientWriteRequestPromise() {}

  void completeRequest() override {
    promise.setValue();
  }

  void completeRequestWithFail(const SdoError& error) override {
    promise.setException(error);
  }

  void completeRequestWithTimeout() override {
    const SdoError error = SdoError(SdoErrorCode::TIMEOUT);
    promise.setException(error);
  }

  folly::Future<folly::Unit> getFuture() {
    return promise.getFuture();
  }

private:
  folly::Promise<folly::Unit> promise;
};

}

#endif // OCO_SDO_CLIENT_REQUEST_PROMISE_HPP


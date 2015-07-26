// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_REQUEST_CALLBACK_HPP
#define OCO_SDO_CLIENT_REQUEST_CALLBACK_HPP

#include "hlcanopen/sdo_client_request.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include <folly/futures/Try.h>

#include <thread>
#include <future>
#include <cstdint>
#include <functional>

namespace hlcanopen {


template<class T> class SdoClientReadRequestCallback : public SdoClientReadRequest {
public:
  SdoClientReadRequestCallback(SDOIndex sdoIndex, std::function<void(folly::Try<T>)> callback,
			       long timeout) :
    SdoClientReadRequest(sdoIndex, timeout),
    callback(callback) {}

  ~SdoClientReadRequestCallback() {};

  virtual void completeRequest(const SdoData& data) override {
    T v = convertSdoData<T>(data);
    callCallbackWithValue(v);
  }

  virtual void completeRequestWithFail(const SdoError& err) override  {
    callCallbackWithException<>(err);
  }

  virtual void completeRequestWithTimeout() override  {
    const SdoError err = SdoError(TIMEOUT);
    callCallbackWithException<>(err);
  }

protected:
  void callCallbackWithValue(const T value) {
    folly::Try<T> t(value);
    callback(t);
  };

  template <typename E> void callCallbackWithException(const E ex) {
    folly::Try<T> t(folly::make_exception_wrapper<E>(ex));
    callback(t);
  }

private:
  std::function<void(folly::Try<T>)> callback;
};

class SdoClientWriteRequestCallback : public SdoClientWriteRequest {
public:
  SdoClientWriteRequestCallback(SDOIndex sdoIndex, SdoData sdoData,
                                   std::function<void(folly::Try<folly::Unit> )> callback,
				   long timeout) :
                                SdoClientWriteRequest(sdoIndex, sdoData, timeout),
                                callback(callback) {}
  virtual ~SdoClientWriteRequestCallback() {}

  void completeRequest() override {
    callback(folly::Try<folly::Unit>(folly::Unit()));
  }
  void completeRequestWithFail(const SdoError& err) override {
    folly::Try<folly::Unit> t(folly::make_exception_wrapper<SdoError>(err));
    callback(t);
  }
  void completeRequestWithTimeout() override {
    folly::Try<folly::Unit> t(folly::make_exception_wrapper<SdoError>(SdoError(TIMEOUT)));
    callback(t);
  }

private:
  std::function<void(folly::Try<folly::Unit>)> callback;
};

}

#endif // OCO_SDO_CLIENT_REQUEST_CALLBACK_HPP


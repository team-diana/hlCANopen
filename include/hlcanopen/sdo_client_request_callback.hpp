// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_REQUEST_CALLBACK_HPP
#define OCO_SDO_CLIENT_REQUEST_CALLBACK_HPP

#include "hlcanopen/sdo_client_request.hpp"
#include "hlcanopen/sdo_response.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include <thread>
#include <future>
#include <cstdint>
#include <functional>

namespace hlcanopen {


template<class T> class SdoClientReadRequestCallback : public SdoClientReadRequest {
public:
  SdoClientReadRequestCallback(SDOIndex sdoIndex, std::function<void(SdoResponse<T> v)> callback,
			       long timeout) :
    SdoClientReadRequest(sdoIndex, timeout),
    callback(callback) {}
  ~SdoClientReadRequestCallback() {};

  virtual void completeRequest(const SdoData& data) override {
    T v = convertSdoData<T>(data);
    SdoResponse<T> sdoResponse(v);
    callCallback(sdoResponse);
  }

  virtual void completeRequestWithFail(const SdoError& error) override  {
    SdoResponse<T> sdoResponse(T(), error);
    callCallback(sdoResponse);
  }

  virtual void completeRequestWithTimeout() override  {
    const SdoError& err = SdoError(TIMEOUT);
    const SdoResponse<T> sdoResponse(T(), err);
    callCallback(sdoResponse);
  }
  
protected:
  void callCallback(const SdoResponse<T>& sdoResponse) {
    callback(sdoResponse);
  };

private:
  std::function<void(SdoResponse<T>)> callback;
};

class SdoClientWriteRequestCallback : public SdoClientWriteRequest {
public:
  SdoClientWriteRequestCallback(SDOIndex sdoIndex, SdoData sdoData,
                                   std::function<void(SdoResponse<bool> v)> callback,
				   long timeout) :
                                SdoClientWriteRequest(sdoIndex, sdoData, timeout),
                                callback(callback) {}
  virtual ~SdoClientWriteRequestCallback() {}

  void completeRequest() override {
    callback(SdoResponse<bool>(true));
  }
  void completeRequestWithFail(const SdoError& error) override {
    callback(SdoResponse<bool>(false, error));
  }
  void completeRequestWithTimeout() override {
    callback(SdoResponse<bool>(false, SdoError(TIMEOUT))); /* XXX */
  }

private:
  std::function<void(SdoResponse<bool> v)> callback;
};

}

#endif // OCO_SDO_CLIENT_REQUEST_CALLBACK_HPP


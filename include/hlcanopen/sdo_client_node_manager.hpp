// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_NODE_MANAGER_H
#define OCO_SDO_CLIENT_NODE_MANAGER_H

#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/sdo_client_request.hpp"
#include "hlcanopen/sdo_client_request_callback.hpp"
#include "hlcanopen/sdo_client_request_promise.hpp"
#include "hlcanopen/object_dictionary.hpp"

#include "boost/assert.hpp"

#include <memory>
#include <queue>
#include <functional>
#include <mutex>
#include <utility>

namespace hlcanopen {

template <class C> class SdoClientNodeManager : public SdoRequestVisitor {

public:
  SdoClientNodeManager(NodeId nodeId, C& card, ObjectDictionary& objDict) :
    nodeId(nodeId),
    card(card),
    objDict(objDict),
    sdoClient(nodeId, card) {}

  // Message arrived from server
  // this is ok both for read from server and write to server
  // for write, the request will have return type void
  void handleSdoTransmit(const CanMsg& m) {
    sdoClient.newMsg(m);
    if(sdoClient.transmissionIsEnded()) {
      sdoClientRequests.front()->visitEnd(*this);
    }
  }

  template<typename T> std::future<SdoResponse<T>> readSdo(const SDOIndex& sdoIndex) override {
    SdoClientReadRequestPromise<T>* request = new SdoClientReadRequestPromise<T>(sdoIndex);
    std::future<SdoResponse<T>> future = request->getFuture();
    sdoClientRequests.push(std::unique_ptr<SdoClientReadRequestPromise<T>>(request));
    startNextSdoRequestIfPossible();
    return future;
  }

  template<typename T> void readSdo(const SDOIndex& sdoIndex,
                                    std::function<void(SdoResponse<T>)> callback) override {
    std::unique_ptr<SdoClientRequest> request =
        std::make_unique<SdoClientReadRequestCallback<T>>(sdoIndex, callback);
    sdoClientRequests.push(std::move(request));
    startNextSdoRequestIfPossible();
  }

  template<typename T> std::future<SdoResponse<bool>> writeSdo(const SDOIndex& sdoIndex, T value) override {
    SdoData sdoData = convertValue<T>(value);
    SdoClientWriteRequestPromise* request = new SdoClientWriteRequestPromise(sdoIndex, sdoData);
    std::future<SdoResponse<bool>> future = request->getFuture();
    sdoClientRequests.push(std::unique_ptr<SdoClientWriteRequest>(request));
    startNextSdoRequestIfPossible();
    return future;
  }

  template<typename T> void writeSdo(const SDOIndex& sdoIndex, T value,
                                    std::function<void(SdoResponse<bool>)> callback) override {
    SdoData sdoData = convertValue<T>(value);
    std::unique_ptr<SdoClientRequest> request =
        std::make_unique<SdoClientWriteRequestCallback>(sdoIndex, sdoData, callback);
    sdoClientRequests.push(std::move(request));
    startNextSdoRequestIfPossible();
  }

private:
  void startNextSdoRequestIfPossible() {
    if(sdoClientRequests.size() > 0 && sdoClient.getTransStatus() == TransStatus::NO_TRANS)
      startNextSdoRequest();
  }

  void startNextSdoRequest() {
    sdoClientRequests.front()->visitStart(*this);
  }

  void visitSdoClientRequestStart(const SdoClientReadRequest& request) override {
    sdoClient.readFromNode(request.sdoIndex);
  }

  void visitSdoClientRequestStart(const SdoClientWriteRequest& request) override {
    sdoClient.writeToNode(request.sdoIndex, request.sdoData);
  }

  void visitSdoClientRequestEnd(SdoClientReadRequest& request) override {
    TransStatus transStatus = sdoClient.getTransStatus();
    if(transStatus == TransStatus::END_ERR) {
      request.completeRequestWithFail(sdoClient.getSdoError());
    } else if(transStatus == TransStatus::END_OK) {
      request.completeRequest(sdoClient.getResponseData());
    }
    endRequest();
  }

  void visitSdoClientRequestEnd(SdoClientWriteRequest& request) override {
    TransStatus transStatus = sdoClient.getTransStatus();
    if(transStatus == TransStatus::END_ERR) {
      request.completeRequestWithFail(sdoClient.getSdoError());
    } else if(transStatus == TransStatus::END_OK) {
      request.completeRequest();
    }
    endRequest();
  }

  void endRequest() {
      sdoClientRequests.pop();
      sdoClient.cleanForNextRequest();
      startNextSdoRequestIfPossible();
  }

private:
  NodeId nodeId;
  C& card;
  ObjectDictionary& objDict;
  SdoClient<C> sdoClient;
  std::queue<std::unique_ptr<SdoClientRequest>> sdoClientRequests;
};

}

#endif // OCO_SDO_CLIENT_NODE_MANAGER_H


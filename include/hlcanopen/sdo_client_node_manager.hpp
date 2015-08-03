// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_NODE_MANAGER_H
#define OCO_SDO_CLIENT_NODE_MANAGER_H

#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/sdo_client_request.hpp"
#include "hlcanopen/sdo_client_request_callback.hpp"
#include "hlcanopen/sdo_client_request_promise.hpp"
#include "hlcanopen/object_dictionary.hpp"

#include "boost/assert.hpp"

#include <folly/futures/Future.h>

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

    template<typename T> folly::Future<T> readSdo(const SDOIndex& sdoIndex, long timeout = 5000) {
        SdoClientReadRequestPromise<T>* request = new SdoClientReadRequestPromise<T>(sdoIndex, timeout);
        folly::Future<T> future = request->getFuture();
        sdoClientRequests.push(std::unique_ptr<SdoClientReadRequestPromise<T>>(request));
        startNextSdoRequestIfPossible();
        return future;
    }

    template<typename T> void readSdo(const SDOIndex& sdoIndex,
                                      std::function<void(folly::Try<T>)> callback, long timeout = 5000) {
        std::unique_ptr<SdoClientRequest> request =
            std::make_unique<SdoClientReadRequestCallback<T>>(sdoIndex, callback, timeout);
        sdoClientRequests.push(std::move(request));
        startNextSdoRequestIfPossible();
    }

    template<typename T> folly::Future<folly::Unit> writeSdo(const SDOIndex& sdoIndex, T value,
            long timeout = 5000) {
        SdoData sdoData = convertValue<T>(value);
        SdoClientWriteRequestPromise* request = new SdoClientWriteRequestPromise(sdoIndex, sdoData, timeout);
        folly::Future<folly::Unit> future = request->getFuture();
        sdoClientRequests.push(std::unique_ptr<SdoClientWriteRequest>(request));
        startNextSdoRequestIfPossible();
        return future;
    }

    template<typename T> void writeSdo(const SDOIndex& sdoIndex, T value,
                                       std::function<void(folly::Try<folly::Unit>)> callback, long timeout = 5000) {
        SdoData sdoData = convertValue<T>(value);
        std::unique_ptr<SdoClientRequest> request =
            std::make_unique<SdoClientWriteRequestCallback>(sdoIndex, sdoData, callback, timeout);
        sdoClientRequests.push(std::move(request));
        startNextSdoRequestIfPossible();
    }

    void updateQueue() {
//    while ( && !isValid(sdoClientRequests.front()->timestamp)) {
        while (true) {
            if (sdoClientRequests.empty()) {
                return;
            }
            if (isValid(sdoClientRequests.front()->timestamp)) {
                return;
            }
            sdoClientRequests.front()->visitTimeout(*this);
        }
    }

private:
    void startNextSdoRequestIfPossible() {
        if(sdoClientRequests.size() > 0 && sdoClient.getTransStatus() == TransStatus::NO_TRANS) {
            startNextSdoRequest();
        }
    }

    void startNextSdoRequest() {
        CLOG(INFO, "sdo") << "starting new sdo request";
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
            CLOG(INFO, "sdo") << "end sdo request with ERR";
            request.completeRequestWithFail(sdoClient.getSdoError());
        } else if(transStatus == TransStatus::END_OK) {
            CLOG(INFO, "sdo") << "end sdo request with OK";
            request.completeRequest(sdoClient.getResponseData());
        }
        endRequest();
    }

    void visitSdoClientRequestTimeout(SdoClientReadRequest& request) override {
        CLOG(INFO, "sdo") << "end sdo request with TIMEOUT";
        request.completeRequestWithTimeout();
        endRequest();
    }

    void visitSdoClientRequestEnd(SdoClientWriteRequest& request) override {
        TransStatus transStatus = sdoClient.getTransStatus();
        if(transStatus == TransStatus::END_ERR) {
            request.completeRequestWithFail(sdoClient.getSdoError());
        } else if(transStatus == TransStatus::END_TIMEOUT) { /* XXX */
            request.completeRequestWithTimeout();
        } else if(transStatus == TransStatus::END_OK) {
            request.completeRequest();
        }
        endRequest();
    }

    void visitSdoClientRequestTimeout(SdoClientWriteRequest& request) override {
        request.completeRequestWithTimeout();
        endRequest();
    }

    void endRequest() {
        sdoClientRequests.pop();
        sdoClient.cleanForNextRequest();
        startNextSdoRequestIfPossible();
    }

    bool isValid(long timestamp) {
        return getTimestamp() < timestamp;
    }

    long getTimestamp() {
        auto ts = std::chrono::duration_cast< std::chrono::milliseconds >(
                      std::chrono::system_clock::now().time_since_epoch())
                  .count();
        return ts;
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


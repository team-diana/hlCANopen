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

class SdoClientNodeManager : public SdoRequestVisitor {

public:
    SdoClientNodeManager(NodeId nodeId, CanCard& card, ObjectDictionary& objDict);

    void handleSdoTransmit(const CanMsg& m);

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

    void updateQueue();

private:
    void startNextSdoRequestIfPossible();
    void startNextSdoRequest();
    void visitSdoClientRequestStart(const SdoClientReadRequest& request) override;
    void visitSdoClientRequestStart(const SdoClientWriteRequest& request) override;
    void visitSdoClientRequestEnd(SdoClientReadRequest& request) override;
    void visitSdoClientRequestTimeout(SdoClientReadRequest& request) override;
    void visitSdoClientRequestEnd(SdoClientWriteRequest& request) override;
    void visitSdoClientRequestTimeout(SdoClientWriteRequest& request) override;
    void endRequest();
    bool isValid(long timestamp);
    long getTimestamp();

private:
    NodeId nodeId;
    CanCard& card;
    ObjectDictionary& objDict;
    SdoClient sdoClient;
    std::queue<std::unique_ptr<SdoClientRequest>> sdoClientRequests;
};

}

#endif // OCO_SDO_CLIENT_NODE_MANAGER_H


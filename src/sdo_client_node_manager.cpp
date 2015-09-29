// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/sdo_client_node_manager.hpp"

namespace hlcanopen {

    SdoClientNodeManager::SdoClientNodeManager(NodeId nodeId, CanCard& card, ObjectDictionary& objDict) :
        nodeId(nodeId),
        card(card),
        objDict(objDict),
        sdoClient(nodeId, card) {}

    // Message arrived from server
    // this is ok both for read from server and write to server
    // for write, the request will have return type void
    void SdoClientNodeManager::handleSdoTransmit(const CanMsg& m) {
        sdoClient.newMsg(m);
        if(sdoClient.transmissionIsEnded()) {
            sdoClientRequests.front()->visitEnd(*this);
        }
    }


    void SdoClientNodeManager::updateQueue() {
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

    void SdoClientNodeManager::startNextSdoRequestIfPossible() {
        if(sdoClientRequests.size() > 0 && sdoClient.getTransStatus() == TransStatus::NO_TRANS) {
            startNextSdoRequest();
        }
    }

    void SdoClientNodeManager::startNextSdoRequest() {
        CLOG(INFO, "sdo") << "starting new sdo request";
        sdoClientRequests.front()->visitStart(*this);
    }

    void SdoClientNodeManager::visitSdoClientRequestStart(const SdoClientReadRequest& request) {
        sdoClient.readFromNode(request.sdoIndex);
    }

    void SdoClientNodeManager::visitSdoClientRequestStart(const SdoClientWriteRequest& request) {
        sdoClient.writeToNode(request.sdoIndex, request.sdoData);
    }

    void SdoClientNodeManager::visitSdoClientRequestEnd(SdoClientReadRequest& request) {
        TransStatus transStatus = sdoClient.getTransStatus();
        if(transStatus == TransStatus::END_ERR) {
            CLOG(INFO, "sdo") << "end sdo read request with ERR";
            request.completeRequestWithFail(sdoClient.getSdoError());
        } else if(transStatus == TransStatus::END_OK) {
            CLOG(INFO, "sdo") << "end sdo read request with OK";
            request.completeRequest(sdoClient.getResponseData());
        }
        endRequest();
    }

    void SdoClientNodeManager::visitSdoClientRequestTimeout(SdoClientReadRequest& request) {
        CLOG(INFO, "sdo") << "end sdo request with TIMEOUT";
        request.completeRequestWithTimeout();
        endRequest();
    }

    void SdoClientNodeManager::visitSdoClientRequestEnd(SdoClientWriteRequest& request) {
        TransStatus transStatus = sdoClient.getTransStatus();
        if(transStatus == TransStatus::END_ERR) {
            CLOG(WARNING, "sdo") << "end sdo write request with ERROR";
            request.completeRequestWithFail(sdoClient.getSdoError());
        } else if(transStatus == TransStatus::END_TIMEOUT) { /* XXX */
            CLOG(WARNING, "sdo") << "end sdo write request with TIMEOUT";
            request.completeRequestWithTimeout();
        } else if(transStatus == TransStatus::END_OK) {
            CLOG(INFO, "sdo") << "end sdo write request with OK";
            request.completeRequest();
        }
        endRequest();
    }

    void SdoClientNodeManager::visitSdoClientRequestTimeout(SdoClientWriteRequest& request) {
        request.completeRequestWithTimeout();
        endRequest();
    }

    void SdoClientNodeManager::endRequest() {
        sdoClientRequests.pop();
        sdoClient.cleanForNextRequest();
        startNextSdoRequestIfPossible();
    }

    bool SdoClientNodeManager::isValid(long timestamp) {
        return getTimestamp() < timestamp;
    }

    long SdoClientNodeManager::getTimestamp() {
        auto ts = std::chrono::duration_cast< std::chrono::milliseconds >(
                      std::chrono::system_clock::now().time_since_epoch())
                  .count();
        return ts;
    }

}


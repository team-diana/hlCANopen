// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_SERVER_NODE_MANAGER_HPP
#define OCO_SDO_SERVER_NODE_MANAGER_HPP

#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/sdo_server_request.hpp"

#include "boost/assert.hpp"

#include <memory>
#include <map>
#include <functional>
#include <mutex>
#include <utility>

namespace hlcanopen {

class SdoServerNodeManager {

public:
    SdoServerNodeManager(NodeId nodeId, CanCard& card, ObjectDictionary& objDict) :
        nodeId(nodeId),
        card(card),
        objDict(objDict) {}

    // Message received from client
    void handleSdoReceive(const CanMsg& msg) {
        if(currentRequest == nullptr) {
            currentRequest = std::make_unique<SdoServerRequest>(nodeId, card, objDict, msg);
        } else {
            currentRequest->newMsg(msg);
        }
        if(currentRequest->isCompleted()) {
            currentRequest = nullptr;
        }
    }

private:
    NodeId nodeId;
    CanCard& card;
    ObjectDictionary& objDict;
    std::unique_ptr<SdoServerRequest> currentRequest;
//   std::map<SDOIndex, std::unique_ptr<SdoServerRequest<C>>, SDOIndexCompare> requestsMap;
};

}

#endif // OCO_SDO_SERVER_NODE_MANAGER_HPP


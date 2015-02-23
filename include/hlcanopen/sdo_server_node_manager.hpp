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

template <class C> class SdoServerNodeManager {

public:
  SdoServerNodeManager(NodeId nodeId, C& card, ObjectDictionary& objDict) :
    nodeId(nodeId),
    card(card),
    objDict(objDict) {}

  // Message received from client
  void handleSdoReceive(const CanMsg& msg) {
    SDOIndex sdoIndex = getSdoIndex(msg);
    if(requestsMap.find(sdoIndex) == requestsMap.end()) {
      requestsMap.emplace(sdoIndex, std::make_unique<SdoServerRequest<C>>(nodeId, card, objDict, msg));
    } else {
      SdoServerRequest<C>* request = requestsMap[sdoIndex].get();
      request->newMsg(msg);
      if(request->isCompleted()) {
        requestsMap.erase(sdoIndex);
      }
    }
  }

private:
  NodeId nodeId;
  C& card;
  ObjectDictionary& objDict;
  std::map<SDOIndex, std::unique_ptr<SdoServerRequest<C>>, SDOIndexCompare> requestsMap;
};

}

#endif // OCO_SDO_SERVER_NODE_MANAGER_HPP


// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_NODE_MANAGER_H
#define OCO_NODE_MANAGER_H

#include "hlcanopen/sdo_client_node_manager.hpp"
#include "hlcanopen/sdo_server_node_manager.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/sdo_client_request_callback.hpp"
#include "hlcanopen/sdo_client_request_promise.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/can_msg.hpp"

#include "boost/assert.hpp"

#include <folly/futures/Future.h>

#include <memory>
#include <queue>
#include <functional>
#include <mutex>
#include <utility>

namespace hlcanopen {

enum class NodeManagerType {
  SERVER, CLIENT
};

template <class C> class NodeManager {
public:
  NodeManager(NodeId nodeId, C& card, NodeManagerType type) :
  nodeId(nodeId),
  managerType(type),
  sdoClientNodeManager(nullptr),
  sdoServerNodeManager(nullptr) {
    if(type == NodeManagerType::CLIENT) {
      sdoClientNodeManager = std::make_unique<SdoClientNodeManager<C>>(nodeId, card, objDict);
    } else {
      sdoServerNodeManager = std::make_unique<SdoServerNodeManager<C>>(nodeId, card, objDict);
    }
  }

  virtual ~NodeManager() {}

  void newMsg(const CanMsg& m) {
    COBType cobType = m.cobId.getCobType();

    if(m.cobId.getCobIdValue() == 0 &&
      m.data == 0) {
      return;
    }

    switch(cobType) {
      case COBType::SDO_TRANSMIT:
        if(managerType == NodeManagerType::CLIENT) {
          sdoClientNodeManager->handleSdoTransmit(m);
        }
        break;
      case COBType::SDO_RECEIVE:
        if(managerType == NodeManagerType::SERVER) {
          sdoServerNodeManager->handleSdoReceive(m);
        }
        break;
      default:
        CLOG(WARNING, "canopen_manager") << "NOT IMPLEMENTED COB TYPE: " << cobType;
        CLOG(DEBUG, "canopen_manager") << "Unknown message dump: " << m;
    }
  }

  template<typename T> T readSdoLocal(SDOIndex sdoIndex) {
    ODEntryValue value = objDict.read(sdoIndex);
    ASSERT_MSG_COUT(typeid(T) != value.type(), "Wrong type for " << sdoIndex);
    return boost::get<T>(value);
  }

  template<typename T> void writeSdoLocal(SDOIndex sdoIndex, T value) {
    objDict.write(sdoIndex, ODEntryValue(value));
  }

  void setSdoAccessLocal(SDOIndex sdoIndex, EntryAccess access) {
    objDict.setAccess(sdoIndex, access);
  }

  template<typename T> folly::Future<T> readSdoRemote(const SDOIndex& sdoIndex, long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    return sdoClientNodeManager->template readSdo<T>(sdoIndex, timeout);
  }

  template<typename T> void readSdoRemote(const SDOIndex& sdoIndex,
                                            std::function<void(folly::Try<T>)> callback, long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    return sdoClientNodeManager->template readSdo<T>(sdoIndex, callback, timeout);
  }

  template<typename T> folly::Future<folly::Unit> writeSdoRemote(const SDOIndex& sdoIndex, T data,
								     long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    return sdoClientNodeManager->template writeSdo<T>(sdoIndex, data, timeout);
  }

  template<typename T> void writeSdoRemote(const SDOIndex& sdoIndex, T data,
                                            std::function<void(folly::Try<folly::Unit>)> callback,
					    long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    sdoClientNodeManager->template writeSdo<T>(sdoIndex, data, callback, timeout);
  }

  void updateQueue() {
//     if (sdoClientNodeManager == nullptr)
//       std::cout << "NULL pointer" << std::endl;
//     else sdoClientNodeManager->updateQueue();
    if (sdoClientNodeManager != nullptr) {
      sdoClientNodeManager->updateQueue();
    }
  }

private:
  void assertType(NodeManagerType type) {
    ASSERT_MSG_COUT(managerType != type, "This node is configured as " <<
      (type==NodeManagerType::CLIENT ? "CLIENT" : "SERVER" ) << ". This method is not supported" );
  }

private:
  NodeId nodeId;
  ObjectDictionary objDict;
  NodeManagerType managerType;
  std::unique_ptr<SdoClientNodeManager<C>> sdoClientNodeManager;
  std::unique_ptr<SdoServerNodeManager<C>> sdoServerNodeManager;
};

}

#endif // OCO_NODE_MANAGER_H


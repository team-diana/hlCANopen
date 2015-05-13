// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_NODE_MANAGER_H
#define OCO_NODE_MANAGER_H

#include "hlcanopen/sdo_client_node_manager.hpp"
#include "hlcanopen/sdo_server_node_manager.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/sdo_client_request_callback.hpp"
#include "hlcanopen/sdo_client_request_promise.hpp"
#include "hlcanopen/pdo_client.hpp"
#include "hlcanopen/pdo_configuration.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/can_msg.hpp"

#include "boost/assert.hpp"

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
  sdoServerNodeManager(nullptr),
  pdoClient(nullptr) {
    if(type == NodeManagerType::CLIENT) {
      sdoClientNodeManager = std::make_shared<SdoClientNodeManager<C>>(nodeId, card, objDict);
      pdoClient = std::make_unique<PdoClient<C>>(nodeId, card, sdoClientNodeManager, objDict);
    }
    else
      sdoServerNodeManager = std::make_unique<SdoServerNodeManager<C>>(nodeId, card, objDict);
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
        NOT_IMPLEMENTED_YET;
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

  template<typename T> std::future<SdoResponse<T>> readSdoRemote(const SDOIndex& sdoIndex, long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    return sdoClientNodeManager->template readSdo<T>(sdoIndex, timeout);
  }

  template<typename T> void readSdoRemote(const SDOIndex& sdoIndex,
                                            std::function<void(SdoResponse<T>)> callback, long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    return sdoClientNodeManager->template readSdo<T>(sdoIndex, callback, timeout);
  }

  template<typename T> std::future<SdoResponse<bool>> writeSdoRemote(const SDOIndex& sdoIndex, T data,
								     long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    return sdoClientNodeManager->template writeSdo<T>(sdoIndex, data, timeout);
  }

  template<typename T> void writeSdoRemote(const SDOIndex& sdoIndex, T data,
                                            std::function<void(SdoResponse<bool>)> callback,
					    long timeout = 5000) {
    assertType(NodeManagerType::CLIENT);
    sdoClientNodeManager->template writeSdo<T>(sdoIndex, data, callback, timeout);
  }

  void writePdoConfiguration(PdoConfiguration config) {
    assertType(NodeManagerType::CLIENT);
    pdoClient->writeConfiguration(config);
  }
  
  void writeRPDO(COBId cobId) {
    assertType(NodeManagerType::CLIENT);
    pdoClient->writeRPDO(cobId);
  }

  void receiveTPDO(CanMsg& m) { /* TODO: this should be in newMsg() */
    assertType(NodeManagerType::CLIENT);
    pdoClient->receiveTPDO(m);
  }
  
  void updateQueue() {
    if (sdoClientNodeManager == nullptr)
      std::cout << "NULL pointer" << std::endl;
    else sdoClientNodeManager->updateQueue();
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
  std::shared_ptr<SdoClientNodeManager<C>> sdoClientNodeManager;
  std::unique_ptr<SdoServerNodeManager<C>> sdoServerNodeManager;
  std::unique_ptr<PdoClient<C>> pdoClient;
};

}

#endif // OCO_NODE_MANAGER_H


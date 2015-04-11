// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_CAN_OPEN_MANAGER_HPP
#define OCO_CAN_OPEN_MANAGER_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/node_manager.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/utils.hpp"

#include "logging/easylogging++.h"

#include <chrono>
#include <thread>
#include <mutex>

namespace hlcanopen {
template<class C> class CanOpenManager {

public:
  CanOpenManager(C& card, std::chrono::milliseconds sleepInterval = std::chrono::milliseconds(150))  :
  card(card),
  running(false),
  intervalSleepTime(sleepInterval)
  {}

  void run() {
    running = true;
    while(running) {
      CanMsg newMsg = card.read();
      NodeId nodeId = newMsg.cobId.getNodeId();
      if(nodeId == 0) {
        // broadcast
      } else if(nodeManagers.find(nodeId) == nodeManagers.end()) {
        LOG(WARNING) << "Received msg with unknown node id: " << nodeId;
      } else {
        nodeManagers[nodeId]->newMsg(newMsg);
      }
      std::this_thread::sleep_for(intervalSleepTime);
    }
  }

  template<typename T> T readSdoLocal(NodeId nodeId, SDOIndex sdoIndex) {
    assertNodeExist(nodeId);
    std::unique_lock<std::mutex> lock(mutex);
    return nodeManagers[nodeId]->template readSdoLocal<T>(sdoIndex);
  }

  template<typename T> void writeSdoLocal(NodeId nodeId, SDOIndex sdoIndex, T value) {
    assertNodeExist(nodeId);
    std::unique_lock<std::mutex> lock(mutex);
    return nodeManagers[nodeId]->writeSdoLocal(sdoIndex, value);
  }

  template<typename T> std::future<SdoResponse<T>> readSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex) {
    initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
    std::unique_lock<std::mutex> lock(mutex);
    return nodeManagers[nodeId]->template readSdoRemote<T>(sdoIndex);
  }

  template<typename T> void readSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex,
                                            std::function<void(SdoResponse<T>)> callback) {
    initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
    std::unique_lock<std::mutex> lock(mutex);
    nodeManagers[nodeId]->template readSdoRemote<T>(sdoIndex, callback);
  }

  void initNode(NodeId nodeId, NodeManagerType type) {
    nodeManagers.emplace(nodeId, std::make_unique<NodeManager<C>>(nodeId, card, type));
  }

  void stop() {
    running = false;
  }

private:
  void assertNodeExist(NodeId nodeId) {
    ASSERT_MSG_COUT(nodeManagers.find(nodeId) == nodeManagers.end(),
                    "No node with id: " << nodeId << " was created yet");
  }

  void initNodeIfNonExistent(NodeId nodeId, NodeManagerType type) {
    nodeManagers.emplace(nodeId, std::make_unique<NodeManager<C>>(nodeId, card, type));
  }


private:
  std::map<NodeId, std::unique_ptr<NodeManager<C>>> nodeManagers;
  C card;
  bool running;
  std::chrono::milliseconds intervalSleepTime;
  std::mutex mutex;
};
}

#endif // OCO_CAN_OPEN_MANAGER_HPP


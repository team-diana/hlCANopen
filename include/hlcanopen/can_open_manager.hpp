// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_CAN_OPEN_MANAGER_HPP
#define OCO_CAN_OPEN_MANAGER_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/node_manager.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/utils.hpp"

#include "hlcanopen/logging/easylogging++.h"
#include "hlcanopen/logging/logging_conf_loader.hpp"

#include <folly/futures/Future.h>

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

  virtual ~CanOpenManager() {}

  void setupLogging() {
    setupLoggingUsingConfigurationDir("logging_conf");
  }

  void setupLoggingUsingConfigurationDir(const char* dirname) {
    loadConfigFilesFromDir(dirname);
  }

  void run() {
    running = true;

    auto timeoutWatchdogThread = std::thread([&]() {
      while(running) {
        std::this_thread::sleep_for(intervalSleepTime);
        {
          std::unique_lock<std::mutex> lock(mutex);
          for (auto const &it : nodeManagers) {
            it.second->updateQueue();
          }
        }
      }
    });

    while(running) {
      CanMsg newMsg = card.read();
      {
        std::unique_lock<std::mutex> lock(mutex);
        NodeId nodeId = newMsg.cobId.getNodeId();
        if(nodeId == 0) {
          // broadcast
        } else if(nodeManagers.find(nodeId) == nodeManagers.end()) {
          CLOG(WARNING, "canopen_manager") << "Received msg with unknown node id: " << nodeId;
        } else {
          nodeManagers[nodeId]->newMsg(newMsg);
        }
      }

      std::this_thread::sleep_for(intervalSleepTime);
    }

    timeoutWatchdogThread.join();
  }


  void resetCommunication(u_int8_t nodeId)
  {
      sendNMT(NMT_RESET_COMMUNICATION, nodeId);
  }

  void resetNode(u_int8_t nodeId)
  {
      sendNMT(NMT_RESET_NODE, nodeId);
  }

  void startRemoteNode(u_int8_t nodeId)
  {
      sendNMT(NMT_START_REMOTE_NODE, nodeId);
  }

  void sendNMT(uint8_t command, uint8_t CANid)
  {
      CanMsg msg;

      msg.cobId = COBId(0, 0);
      msg[0] = command;
      msg[1] = CANid;

      CLOG(INFO, "canopen_manager") << "sending NMT message " << command << " to canId: " << CANid;

      card.write(msg);
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

  void setSdoAccessLocal(NodeId nodeId, SDOIndex sdoIndex, EntryAccess access) {
    nodeManagers[nodeId]->setSdoAccessLocal(sdoIndex, access);
  }

  template<typename T> folly::Future<T> readSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex,
								 long timeout = 5000) {
    initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
    std::unique_lock<std::mutex> lock(mutex);
    return nodeManagers[nodeId]->template readSdoRemote<T>(sdoIndex, timeout);
  }

  template<typename T> void readSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex,
                                            std::function<void(folly::Try<T>)> callback,
					    long timeout = 5000) {
    initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
    std::unique_lock<std::mutex> lock(mutex);
    nodeManagers[nodeId]->template readSdoRemote<T>(sdoIndex, callback, timeout);
  }

  template<typename T> folly::Future<folly::Unit> writeSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex, T value,
								     long timeout = 5000) {
    initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
    std::unique_lock<std::mutex> lock(mutex);
    return nodeManagers[nodeId]->template writeSdoRemote<T>(sdoIndex, value, timeout);
  }

  template<typename T> void writeSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex, T value,
                                            std::function<void(folly::Try<folly::Unit>)> callback,
					    long timeout = 5000) {
    initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
    std::unique_lock<std::mutex> lock(mutex);
    nodeManagers[nodeId]->template writeSdoRemote<T>(sdoIndex, value, callback, timeout);
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


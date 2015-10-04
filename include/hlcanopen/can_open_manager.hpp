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

class CanOpenManager {

public:
    CanOpenManager(CanCard& card, std::chrono::milliseconds sleepInterval = std::chrono::milliseconds(150));
    virtual ~CanOpenManager();
    void setupLogging();
    void setupLoggingUsingConfigurationDir(const char* dirname);
    void setDefaultFutureExecutor(std::shared_ptr<folly::Executor> ex);
    void run();
    void resetCommunication(u_int8_t nodeId);
    void resetNode(u_int8_t nodeId);
    void startRemoteNode(u_int8_t nodeId);
    void sendNMT(uint8_t command, uint8_t CANid);
    void setSdoAccessLocal(NodeId nodeId, SDOIndex sdoIndex, EntryAccess access);

    template<typename T> T readSdoLocal(NodeId nodeId, SDOIndex sdoIndex) {
        assertNodeExist(nodeId);
        std::unique_lock<std::mutex> lock(mutex);
        return nodeManagers[nodeId]->readSdoLocal<T>(sdoIndex);
    }

    template<typename T> void writeSdoLocal(NodeId nodeId, SDOIndex sdoIndex, T value) {
        assertNodeExist(nodeId);
        std::unique_lock<std::mutex> lock(mutex);
        return nodeManagers[nodeId]->writeSdoLocal(sdoIndex, value);
    }

    template<typename T> folly::Future<T> readSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex,
            long timeout = 5000) {
        initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
        std::unique_lock<std::mutex> lock(mutex);
        folly::Future<T> fut = nodeManagers[nodeId]->readSdoRemote<T>(sdoIndex, timeout);
        return addExecutorIfAny(std::move(fut));
    }

    template<typename T> void readSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex,
                                            std::function<void(folly::Try<T>)> callback,
                                            long timeout = 5000) {
        initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
        std::unique_lock<std::mutex> lock(mutex);
        nodeManagers[nodeId]->readSdoRemote<T>(sdoIndex, callback, timeout);
    }

    template<typename T> folly::Future<folly::Unit> writeSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex, T value,
            long timeout = 5000) {
        initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
        std::unique_lock<std::mutex> lock(mutex);
        folly::Future<folly::Unit> fut =  nodeManagers[nodeId]->writeSdoRemote<T>(sdoIndex, value, timeout);
        return addExecutorIfAny(std::move(fut));
    }

    template<typename T> void writeSdoRemote(NodeId nodeId, const SDOIndex& sdoIndex, T value,
            std::function<void(folly::Try<folly::Unit>)> callback,
            long timeout = 5000) {
        initNodeIfNonExistent(nodeId, NodeManagerType::CLIENT);
        std::unique_lock<std::mutex> lock(mutex);
        nodeManagers[nodeId]->writeSdoRemote<T>(sdoIndex, value, callback, timeout);
    }

    void initNode(NodeId nodeId, NodeManagerType type);
    void stop();

private:
    void assertNodeExist(NodeId nodeId);
    void initNodeIfNonExistent(NodeId nodeId, NodeManagerType type);

    template<typename T> folly::Future<T> addExecutorIfAny(folly::Future<T>&& fut) {
        if(executor) {
            return fut.via(executor.get());
        } else {
            return std::move(fut);
        }
    }


private:
    std::map<NodeId, std::unique_ptr<NodeManager>> nodeManagers;
    CanCard& card;
    bool running;
    std::chrono::milliseconds intervalSleepTime;
    std::mutex mutex;
    std::shared_ptr<folly::Executor> executor;
};

}

#endif // OCO_CAN_OPEN_MANAGER_HPP


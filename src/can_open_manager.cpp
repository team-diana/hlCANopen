// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/can_open_manager.hpp"

namespace hlcanopen {

    CanOpenManager::CanOpenManager(CanCard& card, std::chrono::milliseconds sleepInterval)  :
        card(card),
        running(false),
        intervalSleepTime(sleepInterval)
    {}

    CanOpenManager::~CanOpenManager() {}

    void CanOpenManager::setupLogging() {
        setupLoggingUsingConfigurationDir("logging_conf");
    }

    void CanOpenManager::setupLoggingUsingConfigurationDir(const char* dirname) {
        loadConfigFilesFromDir(dirname);
    }

    void CanOpenManager::setDefaultFutureExecutor(std::shared_ptr<folly::Executor> ex) {
        executor = ex;
    }

    void CanOpenManager::run() {
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


    void CanOpenManager::resetCommunication(u_int8_t nodeId)
    {
        sendNMT(NMT_RESET_COMMUNICATION, nodeId);
    }

    void CanOpenManager::resetNode(u_int8_t nodeId)
    {
        sendNMT(NMT_RESET_NODE, nodeId);
    }

    void CanOpenManager::startRemoteNode(u_int8_t nodeId)
    {
        sendNMT(NMT_START_REMOTE_NODE, nodeId);
    }

    void CanOpenManager::sendNMT(uint8_t command, uint8_t CANid)
    {
        CanMsg msg;

        msg.cobId = COBId(0, 0);
        msg[0] = command;
        msg[1] = CANid;

        CLOG(INFO, "canopen_manager") << "sending NMT message " << std::hex << (int) command << " to canId: " << std::hex << (int) CANid;

        card.write(msg);
    }

    void CanOpenManager::setSdoAccessLocal(NodeId nodeId, SDOIndex sdoIndex, EntryAccess access) {
        nodeManagers[nodeId]->setSdoAccessLocal(sdoIndex, access);
    }

    void CanOpenManager::initNode(NodeId nodeId, NodeManagerType type) {
        nodeManagers.emplace(nodeId, std::make_unique<NodeManager>(nodeId, card, type));
    }

    void CanOpenManager::stop() {
        running = false;
    }

    void CanOpenManager::assertNodeExist(NodeId nodeId) {
        ASSERT_MSG_COUT(nodeManagers.find(nodeId) == nodeManagers.end(),
                        "No node with id: " << nodeId << " was created yet");
    }

    void CanOpenManager::initNodeIfNonExistent(NodeId nodeId, NodeManagerType type) {
        nodeManagers.emplace(nodeId, std::make_unique<NodeManager>(nodeId, card, type));
    }

}

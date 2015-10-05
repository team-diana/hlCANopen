// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/node_manager.hpp"

namespace hlcanopen {

    NodeManager::NodeManager(NodeId nodeId, CanCard& card, NodeManagerType type) :
        nodeId(nodeId),
        managerType(type),
        sdoClientNodeManager(nullptr),
        sdoServerNodeManager(nullptr),
        pdoClient(*this, card) {
        if(type == NodeManagerType::CLIENT) {
            sdoClientNodeManager = std::make_unique<SdoClientNodeManager>(nodeId, card, objDict);
        } else {
            sdoServerNodeManager = std::make_unique<SdoServerNodeManager>(nodeId, card, objDict);
        }
    }

    NodeManager::~NodeManager() {

    }

    void NodeManager::newMsg(const CanMsg& m) {
        COBType cobType = m.cobId.getCobType();

        if(m.cobId.getCobIdValue() == 0 &&
                m.data == 0) {
            return;
        }

        switch(cobType) {
        case COBType::SDO_TRANSMIT:
            if(managerType == NodeManagerType::CLIENT) {
                CLOG(DEBUG, "canopen_manager") << "new sdo transmit message";
                sdoClientNodeManager->handleSdoTransmit(m);
            }
            break;
        case COBType::SDO_RECEIVE:
            if(managerType == NodeManagerType::SERVER) {
                CLOG(DEBUG, "canopen_manager") << "new sdo receive message";
                sdoServerNodeManager->handleSdoReceive(m);
            }
            break;
        case COBType::PDO:
            CLOG(DEBUG, "canopen_manager") << "new pdo receive message";
            break;
        default:
            CLOG(WARNING, "canopen_manager") << "NOT IMPLEMENTED COB TYPE: " << cobType;
            CLOG(DEBUG, "canopen_manager") << "Unknown message dump: " << m;
        }
    }

    void NodeManager::setSdoAccessLocal(SDOIndex sdoIndex, EntryAccess access) {
        objDict.setAccess(sdoIndex, access);
    }

    void NodeManager::updateQueue() {
        if (sdoClientNodeManager != nullptr) {
            sdoClientNodeManager->updateQueue();
        }
    }

    void NodeManager::assertType(NodeManagerType type) {
        ASSERT_MSG_COUT(managerType != type, "This node is configured as " <<
                        (type==NodeManagerType::CLIENT ? "CLIENT" : "SERVER" ) << ". This method is not supported" );
    }

}



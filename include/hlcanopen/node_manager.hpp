// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_NODE_MANAGER_H
#define OCO_NODE_MANAGER_H

#include "hlcanopen/sdo_client_node_manager.hpp"
#include "hlcanopen/sdo_server_node_manager.hpp"
#include "hlcanopen/pdo_client.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/sdo_client_request_callback.hpp"
#include "hlcanopen/sdo_client_request_promise.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

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

class NodeManager {
public:
    NodeManager(NodeId nodeId, CanCard& card, NodeManagerType type);
    virtual ~NodeManager();

    void newMsg(const CanMsg& m);

    template<typename T> T readSdoLocal(SDOIndex sdoIndex) {
        ODEntryValue value = objDict.read(sdoIndex);
        ASSERT_MSG_COUT(typeid(T) != value.type(), "Wrong type for " << sdoIndex);
        return boost::get<T>(value);
    }

    template<typename T> void writeSdoLocal(SDOIndex sdoIndex, T value) {
        objDict.write(sdoIndex, ODEntryValue(value));
    }

    void setSdoAccessLocal(SDOIndex sdoIndex, EntryAccess access);

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

    void updateQueue();

private:
    void assertType(NodeManagerType type);

private:
    NodeId nodeId;
    ObjectDictionary objDict;
    NodeManagerType managerType;
    std::unique_ptr<SdoClientNodeManager> sdoClientNodeManager;
    std::unique_ptr<SdoServerNodeManager> sdoServerNodeManager;
    PdoClient<hlcanopen::NodeManager> pdoClient;
};

}

#endif // OCO_NODE_MANAGER_H


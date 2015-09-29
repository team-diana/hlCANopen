// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_HPP
#define OCO_SDO_CLIENT_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_error.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/can_msg_utils.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/can_card.hpp"

#include <experimental/optional>

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include "hlcanopen/logging/easylogging++.h"

namespace hlcanopen {


enum class TransStatus {
    NO_TRANS,
    CONT,
    END_OK,
    END_ERR,
    END_TIMEOUT
};

enum {
    SDO_TRANSMIT_COB_ID = 0b1011,
    SDO_RECEIVE_COB_ID = 0b1100
};

class SdoClient {
    typedef boost::coroutines::asymmetric_coroutine<CanMsg> coroutine;

public:
    SdoClient(NodeId nodeId, CanCard& card);

    void readFromNode(SDOIndex sdoIndex);

    void writeToNode(SDOIndex sdoIndex, const SdoData& data);

    void newMsg(CanMsg msg);

    TransStatus getTransStatus();

    SdoError getSdoError();

    std::vector<unsigned char> getResponseData();

    void cleanForNextRequest();

    bool transmissionIsEnded();

private:
    void startReadFromNodeCoroutine(SDOIndex sdoIndex);

    void writeToNodeExpedited(const SDOIndex sdoIndex, const SdoData& data);

    void startWriteToNodeExpeditedCoroutine(const SDOIndex sdoIndex);

    void writeToNodeSegmented(const SDOIndex sdoIndex, const SdoData& data);

    void startWriteToNodeSegmentedCoroutine(const SDOIndex sdoIndex, const SdoData& data);

    bool isAbortMsg(CanMsg msg);

    COBId makeReqCobId();

private:
    NodeId nodeId;
    CanCard& card;
    std::unique_ptr<coroutine::push_type> sdoCoroutine;
    SdoData receivedData;
    TransStatus currentTransStatus;
    SdoError currentSdoError;

};

}

#endif // OCO_SDO_CLIENT_HPP


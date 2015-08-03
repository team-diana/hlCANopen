// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_PDO_CLIENT_HPP
#define OCO_PDO_CLIENT_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/sdo_error.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/can_msg_utils.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/pdo_configuration.hpp"


#include <boost/coroutine/asymmetric_coroutine.hpp>

#include "hlcanopen/logging/easylogging++.h"

#include <experimental/optional>

namespace hlcanopen {

enum CommunicationParamSubIndex {
    NUMBER_OF_ENTRIES_SUB_INDEX = 0,
    COB_ID_SUB_INDEX = 1,
    TRANSMISSION_TYPE_SUB_INDEX = 2,
    INHIBIT_TIME_SUB_INDEX = 3,
    RESERVED_SUB_INDEX = 4,
    EVENT_TIMER_SUB_INDEX = 5
};

struct PDOEntry {
    PDOEntry() {};
    uint8_t length; // Useless for RPDO, but we use the same struct
    // to have a single PDOMap
    SDOIndex local_object;
};


template<template<typename C> class N, class C> class PdoClient {
    typedef boost::coroutines::asymmetric_coroutine<CanMsg> coroutine;

    // N is NodeManager, but we use a template parameter in order to allow
    // unit testing
    N<C>& nodeManager;
    std::map<COBId, std::vector<PDOEntry>> PDOMap;
    ObjectDictionary od;
    C& card;

public:
    PdoClient(N<C>& nodeManager, C& card) :
      nodeManager(nodeManager),
      card(card)
    {
    }

    auto resetPdoConfiguration(unsigned int index) {
        const SdoData zero_data {
            0, 0, 0, 0
        };

        /* Disable the PDO configuration */
        nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index, COB_ID_SUB_INDEX), 0).wait();
        nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index + 0x200, 0), 0).wait();
    }

    auto setupCommunicationParameters(unsigned int index, const PdoConfiguration& configuration) {
        nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index, TRANSMISSION_TYPE_SUB_INDEX),
                                   configuration.getTransmissionTypeValue()).wait();
        nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index, INHIBIT_TIME_SUB_INDEX),
                                   configuration.getInhibitTime()).wait();
        nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index, RESERVED_SUB_INDEX),
                                   configuration.getReserved()).wait();
        return nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index, EVENT_TIMER_SUB_INDEX),
                                          configuration.getEventTimer()).wait();
    }

    auto setupMapping(unsigned int index, const PdoConfiguration& configuration) {
        auto mapp = configuration.getMap();
        uint8_t i = 1;
        PDOEntry entry;
        int s = mapp.size() + 1; // index 0 is wasted
        std::vector<PDOEntry> vect(s);
        for (auto it = mapp.begin(); it != mapp.end(); it++, i++) {
            entry.length = it->length;
            entry.local_object = it->local;
            nodeManager.template writeSdoRemote<uint32_t> (SDOIndex(index + 0x200, i), it->getValue()).wait();
            vect[it->position] = entry;
        }

        PDOMap[configuration.getCobIdPdo()] = vect;

        return nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index + 0x200, 0), (uint8_t) i - 1).wait();
    }

    void writeConfiguration(PdoConfiguration configuration) {
        unsigned int index = configuration.getPdoNumber() - 1;

        resetPdoConfiguration(index);
        setupCommunicationParameters(index, configuration);
        setupMapping(index, configuration);

        /* Enable the PDO */
        nodeManager.template writeSdoRemote<uint32_t>(SDOIndex(index, COB_ID_SUB_INDEX),
                                   configuration.getCobIdValue()).wait();
    }

    void receiveTPDO(CanMsg& msg) {
        unsigned int pos;
        for (auto entry : PDOMap(msg.cobId)) { // XXX: Is it ordered from index 0 to length()-1?
            SdoData data = getBytesAsData(msg, pos, pos + entry.second.length - 1);
            od.write(entry.second.local, /* cast to ODEntry */ data);
            pos += entry.second.length;
        }
    }

    void writeRPDO(COBId cobId) {
        CanMsg msg;
        msg.cobId = cobId;
        unsigned int pos = 0;
        for (auto entry : PDOMap(msg.cobId)) {
            pos += entry.second.local_object;
            msg.data |= od.read(entry.second.local_object) << (64 - pos);
        }

        card.write(msg);
    }
};

}

#endif // OCO_PDO_CLIENT_HPP


// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_PDO_CONFIGURATION_HPP
#define OCO_PDO_CONFIGURATION_HPP

#include "hlcanopen/types.hpp"

#include "hlcanopen/logging/easylogging++.h"
#include <boost/concept_check.hpp>

#include <list>

namespace hlcanopen {

enum TransmissionType {
    SYNCHRONOUS_CYCLIC,
    N_A,
    SYNCHRONOUS_RTR,
    ASYNCHRONOUS_RTR,
    ASYNCHRONOUS
};

enum {
    MAX_MAPPING_LENGTH = 64
};

typedef uint16_t EventTimer;

struct MappingEntry {
    SDOIndex local;
    SDOIndex remote;
    uint8_t length;
    uint8_t position;

    uint32_t getValue() {
        uint32_t entry = 0x00000000;
        entry |= remote.index << 16;
        entry |= remote.subIndex << 8;
        entry |= length;
        return entry;
    }
};

typedef std::list<MappingEntry> Mapping;

enum PDO_TYPE {
    RPDO = 0x1400,
    TPDO = 0x1800
};

class COBIdPdoEntry {
    COBId cobId;
    bool use29BitId = false;
    bool disableRtr = true;
    bool disablePdo = true;

public:
    COBIdPdoEntry()
    {
    }

    void setCobId(COBId c) {
        cobId = c;
    }

    COBId getCobId() const {
        return cobId;
    }

    void enable29bitId(bool b) {
        use29BitId = b;
    }

    bool is29bitId() const {
        return use29BitId;
    }

    void enableRtr(bool b) {
        disableRtr = !b;
    }

    bool isRtrEnabled() const {
        return !disableRtr;
    }

    void enablePdo(bool b) {
        disablePdo = !b;
    }

    bool isPdoEnabled() const {
        return !disablePdo;
    }

    uint32_t getData() const {
        uint8_t data = 0x00;
        if (disablePdo)
            data |= (1 << 31);
        if (disableRtr)
            data |= (1 << 30);
        if (use29BitId) {
            data |= (1 << 29);
            data |= (cobId.getCobIdValue() & 0b00011111111111111111111111111);
        } else {
            data |= (cobId.getCobIdValue() & 0b00000000000000000011111111111);
        }

        return data;
    }

};

class PdoConfiguration {
    unsigned int numberOfPdo = 0;
    uint8_t numberOfEntries = 2;
    COBIdPdoEntry cobId;
    uint8_t transType = 0xFF;
    uint16_t inhibitTime = 0x0000;
    uint8_t reserved = 0x00;
    EventTimer timer = 0x0000;

    Mapping map;

public:
    PdoConfiguration(PDO_TYPE type, unsigned int number) :
        numberOfPdo(type + number)
    {
    }

    unsigned int getPdoNumber() const {
        return numberOfPdo;
    }

    void setNumberOfEntries() {
        if (timer != 0x0000) {
            numberOfEntries = 5;
        }
        else if (reserved != 0x00) {
            numberOfEntries = 4;
        }
        else if (inhibitTime != 0x0000) {
            numberOfEntries = 3;
        }
        else if (transType != 0x00) {
            numberOfEntries = 2;
        }
    }

    void setNumberOfEntries(uint8_t number) {
        numberOfEntries = number;
    }

    uint8_t getNumberOfEntries() const {
        return numberOfEntries;
    }

    void setCobId(COBIdPdoEntry c) {
        cobId = c;
    }

    COBIdPdoEntry getCobId() const {
        return cobId;
    }

    COBId getCobIdPdo() const {
        return cobId.getCobId();
    }

    uint32_t getCobIdValue() const {
        return cobId.getData();
    }

    void setTransmissionType(TransmissionType type, unsigned int synchronousCyclicInterval = 1) {
        /* TODO: check for offset correctness */
        switch(type) {
        case(SYNCHRONOUS_CYCLIC):
            transType = 0x00;
            transType += synchronousCyclicInterval;
            break;
        case(N_A): // Should we consider this hypothesis?
            break;
        case(SYNCHRONOUS_RTR):
            transType = 0xFC;
            break;
        case(ASYNCHRONOUS_RTR):
            transType = 0xFD;
            break;
        case(ASYNCHRONOUS):
            transType = 0xFE; // What about 0xFF?
            break;
        }
    }

    TransmissionType getTransmissionType() const {
        if (transType < 0xFC)
            return SYNCHRONOUS_CYCLIC;
        if (transType == 0xFC)
            return SYNCHRONOUS_RTR;
        if (transType == 0xFD)
            return ASYNCHRONOUS_RTR;
        if (transType == 0xFE)
            return ASYNCHRONOUS;
        std::runtime_error("unknown TransmissionType");
        return ASYNCHRONOUS;
    }

    uint8_t getTransmissionTypeValue() const {
        return transType;
    }

    unsigned int getSynchronousCyclicInterval() const {
        if (transType < 0xFC)
            return 0;
        return transType - SYNCHRONOUS_CYCLIC;
    }

    void setInhibitTime(uint16_t time) {
        inhibitTime = time;
    }

    uint16_t getInhibitTime() const {
        return inhibitTime;
    }

    void setReserved(uint8_t res) {
        reserved = res;
    }

    uint8_t getReserved() const {
        return reserved;
    }

    void setEventTimer(EventTimer et) {
        timer = et;
    }

    EventTimer getEventTimer() const {
        return timer;
    }

    void addMapping(SDOIndex local, SDOIndex remote, uint8_t length) {
        /* Should we check if the total length is greater than 64 bit? */

        if(getMappingTotalLength() + length > MAX_MAPPING_LENGTH) {
            throw std::runtime_error("mapping length cannot be greater than 8 bytes");
        }

        uint8_t position = map.size() + 1;
        MappingEntry entry {local, remote, length, position};
        map.push_back(entry);
    }

    // return the sum of the length of each entry
    uint getMappingTotalLength() const {
        return std::accumulate(map.begin(), map.end(), 0,
        [](auto a, auto e) {
            return a + e.length;
        });
    }

    Mapping getMap() const {
        return map;
    }


};

}

#endif // OCO_PDO_CONFIGURATION_HPP

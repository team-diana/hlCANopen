// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_CAN_MSG_UTILS_HPP
#define OCO_CAN_MSG_UTILS_HPP

#include "hlcanopen/can_msg.hpp"

namespace hlcanopen {

enum class SdoClientCommandSpecifier : uint8_t {
    DOWNLOAD_SEGMENT = 0,
    INITIATE_DOWNLOAD = 1,
    INITIATE_UPLOAD = 2,
    UPLOAD_SEGMENT = 3,
    ABORT_TRANSFER = 4,
};

enum class SdoServerCommandSpecifier : uint8_t {
    UPLOAD_SEGMENT_RESPONSE = 0,
    DOWNLOAD_SEGMENT_RESPONSE = 1,
    INITIATE_UPLOAD_RESPONSE = 2,
    INITIATE_DOWNLOAD_RESPONSE = 3,
    ABORT_TRANSFER = 4
};


void setSdoIndex(CanMsg& msg, const SDOIndex& sdoIndex);

SDOIndex getSdoIndex(const CanMsg& msg);

// Set the Less Significant 4 byte
void setLast4Byte(CanMsg& msg, uint32_t value);
// Get the Less Significant 4 byte
uint32_t getLast4Byte(const CanMsg& msg);

// Get all the bytes between from (included) and to (included)
SdoData getBytesAsData(const CanMsg& msg, unsigned int from, unsigned int to);

SdoClientCommandSpecifier getSdoClientCommandSpecifier(const CanMsg& msg);
SdoServerCommandSpecifier getSdoServerCommandSpecifier(const CanMsg& msg);
void setSdoClientCommandSpecifier(CanMsg& msg, SdoClientCommandSpecifier spec);
void setSdoServerCommandSpecifier(CanMsg& msg, SdoServerCommandSpecifier spec);

unsigned int getSdoNumberOfBytesOfDataInMsg(const CanMsg& msg);
void setSdoNumberOfBytesOfDataInMsgSdoClient(CanMsg& msg, unsigned int numberOfBytes,
        SdoClientCommandSpecifier commandSpec);
void setSdoNumberOfBytesOfDataInMsgSdoServer(CanMsg& msg, unsigned int numberOfBytes,
        SdoServerCommandSpecifier commandSpec);

bool sdoToggleBitIsSet(const CanMsg& msg);
void setSdoToggleBit(CanMsg& msg, bool on);

bool sdoSizeBitIsSet(const CanMsg& msg);
void setSdoSizeBit(CanMsg& msg, bool on);

bool sdoExpeditedTransferIsSet(const CanMsg& msg);
void setSdoExpeditedTransfer(CanMsg& msg, bool on);

bool sdoNoMoreSegmentBitIsSet(const CanMsg& msg);
void setSdoNoMoreSegmentBit(CanMsg& msg, bool on);

}

#endif // OCO_CAN_MSG_UTILS_HPP


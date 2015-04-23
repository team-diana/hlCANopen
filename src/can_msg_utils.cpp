// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/can_msg_utils.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/utils.hpp"

#include "hlcanopen/logging/easylogging++.h"


namespace hlcanopen {

void setSdoIndex(CanMsg& msg, const SDOIndex& sdoIndex) {
  msg[1] = sdoIndex.index & 0x00ff;
  msg[2] = sdoIndex.index >> 8;
  msg[3] = sdoIndex.subIndex;
}

SDOIndex getSdoIndex(const CanMsg& msg) {
  unsigned int index = (unsigned int)msg[1] << 8 | msg[2];
  return SDOIndex(index, msg[3]);
}

void setLast4Byte(CanMsg& msg, uint32_t value) {
  // TODO: see if it possible to use this
//     uint32_t* last4Byte = reinterpret_cast<uint32_t*>(&msg.data);
//     *last4Byte = value;
    msg[4] = (value & 0x000000ff);
    msg[5] = (value & 0x0000ff00) >> 8;
    msg[6] = (value & 0x00ff0000) >> 16;
    msg[7] = (value & 0xff000000) >> 24;
}

uint32_t getLast4Byte(const CanMsg& msg) {
  return (uint32_t)msg[7] << 24 |
         (uint32_t)msg[6] << 16 |
         (uint32_t)msg[5] << 8  |
                   msg[4];
}

SdoData getBytesAsData(const CanMsg& msg, unsigned int from, unsigned int to)
{
  BOOST_ASSERT_MSG(to>=from && from>=1 && to<=7, "invalid message data access");
  SdoData data;
  data.resize(to-from+1);
  for(int i = 0; from + i <= to; i++) {
      data[i] = msg[from+i];
  }
  return data;
}

SdoClientCommandSpecifier getSdoClientCommandSpecifier(const CanMsg& msg)
{
  return static_cast<SdoClientCommandSpecifier>((msg[0] & 0b1110'0000) >> 5);
}

SdoServerCommandSpecifier getSdoServerCommandSpecifier(const CanMsg& msg)
{
  return static_cast<SdoServerCommandSpecifier>((msg[0] & 0b1110'0000) >> 5);
}

void setSdoClientCommandSpecifier(CanMsg& msg, SdoClientCommandSpecifier spec)
{
  msg[0] &= ~(0b111 << 5);
  msg[0] |= static_cast<uint8_t>(spec) << 5;
}

void setSdoServerCommandSpecifier(CanMsg& msg, SdoServerCommandSpecifier spec)
{
  msg[0] &= ~(0b111 << 5);
  msg[0] |= static_cast<uint8_t>(spec) << 5;
}

unsigned int getSdoNumberOfBytesOfDataInMsg(const CanMsg& msg)
{
  unsigned int NumByteNonContainingData=0;
  unsigned int dataStartsFrom=0;

  if(msg.cobId.getCobType() == COBType::SDO_TRANSMIT) {
    SdoServerCommandSpecifier commandSpec = getSdoServerCommandSpecifier(msg);
    if(commandSpec == SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE) {
      NumByteNonContainingData = (msg[0] & 0b00001100) >> 2;
      dataStartsFrom = 4;
    } else if(commandSpec == SdoServerCommandSpecifier::UPLOAD_SEGMENT_RESPONSE) {
      NumByteNonContainingData = (msg[0] & 0b00001110) >> 1;
      dataStartsFrom = 1;
    } else LOG(ERROR) << "Unexpected commandSpec: " << static_cast<int>(commandSpec);
  } else if(msg.cobId.getCobType() == COBType::SDO_RECEIVE) {
    SdoClientCommandSpecifier commandSpec = getSdoClientCommandSpecifier(msg);
    if(commandSpec == SdoClientCommandSpecifier::INITIATE_DOWNLOAD) {
      NumByteNonContainingData = (msg[0] & 0b00001100) >> 2;
      dataStartsFrom = 4;
    } else if(commandSpec == SdoClientCommandSpecifier::DOWNLOAD_SEGMENT) {
      NumByteNonContainingData = (msg[0] & 0b00001110) >> 1;
      dataStartsFrom = 1;
    } else LOG(ERROR) << "Unexpected commandSpec: " << static_cast<int>(commandSpec);
  } else {
    LOG(ERROR) << "Unexpected COB-ID type: " << msg.cobId.getCobTypeValue();
  }

  return 8 - NumByteNonContainingData - dataStartsFrom;
}

void setSdoNumberOfBytesOfDataInMsgSdoServer(CanMsg& msg, unsigned int numberOfBytes,
                                             SdoServerCommandSpecifier commandSpec)
{
  unsigned int shiftNum = 0;
  unsigned int dataStartsFrom = 0;
  if(commandSpec == SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE) {
    shiftNum = 2;
    dataStartsFrom = 4;
    msg[0] &= ~0b1100;
  } else if (commandSpec == SdoServerCommandSpecifier::UPLOAD_SEGMENT_RESPONSE) {
    msg[0] &= ~0b1110;
    shiftNum = 1;
    dataStartsFrom = 1;
  } else {
    ASSERT_MSG_COUT(false, "Unexpected command spec: " << static_cast<int>(commandSpec) );
  }
  msg[0] |= (8-numberOfBytes-dataStartsFrom) << shiftNum;
}

void setSdoNumberOfBytesOfDataInMsgSdoClient(CanMsg& msg, unsigned int numberOfBytes,
                                             SdoClientCommandSpecifier commandSpec)
{
  int shiftNum = 0;
  int maxBytes =0;
  if(commandSpec == SdoClientCommandSpecifier::INITIATE_DOWNLOAD) {
    shiftNum = 2;
    maxBytes = 4;
  } else if (commandSpec == SdoClientCommandSpecifier::DOWNLOAD_SEGMENT) {
    shiftNum = 1;
    maxBytes = 7;
  } else {
    ASSERT_MSG_COUT(false, "Unexpected command spec: " << static_cast<int>(commandSpec));
  }
  msg[0] = msg[0] | (maxBytes-numberOfBytes) << shiftNum;
}

bool sdoToggleBitIsSet(const CanMsg& msg)
{
  return msg[0] & 0b00010000;
}

void setSdoToggleBit(CanMsg& msg, bool on)
{
  if(on)
    msg[0] |=  0b00010000;
  else
    msg[0] &= ~0b00010000;
}

bool sdoSizeBitIsSet(const CanMsg& msg)
{
  return msg[0] & 0b1;
}

void setSdoSizeBit(CanMsg& msg, bool on)
{
  if(on)
    msg[0] |=  0b1;
  else
    msg[0] &= ~0b1;
}

bool sdoExpeditedTransferIsSet(const CanMsg& msg)
{
  return msg[0] & 0b10;
}

void setSdoExpeditedTransfer(CanMsg& msg, bool on)
{
  if(on)
    msg[0] |=  0b10;
  else
    msg[0] &= ~0b10;
}

bool sdoNoMoreSegmentBitIsSet(const CanMsg& msg)
{
  return msg[0] & 0b1;
}

void setSdoNoMoreSegmentBit(CanMsg& msg, bool on)
{
  if(on)
    msg[0] |=  0b1;
  else
    msg[0] &= ~0b1;
}

}


// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/types.hpp"

namespace hlcanopen {

COBId::COBId() :
  nodeId(0),
  cobTypeValue(0) {}

COBId::COBId(NodeId nodeId, unsigned int cobTypeValue) :
  nodeId(nodeId),
  cobTypeValue(cobTypeValue) {}

NodeId COBId::getNodeId() const
{
  return nodeId;
}

unsigned int COBId::getCobIdValue() const
{
  return cobTypeValue << 7 | nodeId;
}

unsigned int COBId::getCobTypeValue() const
{
  return cobTypeValue;
}

COBType COBId::getCobType() const
{
  switch(cobTypeValue) {
    case 0b0000:
      return COBType::NMT;
    case 0b0001:
      if(nodeId == 0)
        return COBType::SYNC;
      else
        return COBType::EMERGENCY;
    case 0b0010:
      return COBType::TIMESTAMP;
    case 0b1011:
      return COBType::SDO_TRANSMIT;
    case 0b1100:
      return COBType::SDO_RECEIVE;
    case 0b1110:
      return COBType::ERROR_CONTROL;
    default:
      return COBType::PDO;
  }
}

bool COBId::operator==(const COBId& r) const
{
  return nodeId == r.nodeId && cobTypeValue == r.cobTypeValue;
}

bool COBId::operator!=(const COBId& r) const
{
  return !(*this == r);
}

std::ostream& operator<<(std::ostream& out, const COBId& cobId)
{
  return out << "CobId{nodeId=" << cobId.nodeId;
//   return out << "CobId{nodeId=" << cobId.nodeId << ", cobType=" << cobId.getCobType() << "} ";
}

SDOIndex::SDOIndex(unsigned index, unsigned subIndex) : index(index), subIndex(subIndex) {}

bool SDOIndex::operator==(const SDOIndex& r) const
{
  return index == r.index && subIndex == r.subIndex;
}

bool SDOIndex::operator!=(const SDOIndex& r) const
{
  return !(*this==r);
}

std::ostream& operator<<(std::ostream& out, const SDOIndex& sdoIndex)
{
  return out << "SDOIndex{index=" << std::hex << sdoIndex.index;
}

}


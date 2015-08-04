// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/types.hpp"
#include "hlcanopen/utils.hpp"

#include <sstream>

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
std::ostream& operator<< (std::ostream & os, COBType val)
{
    switch (val)
    {
    case COBType::NMT          :
        return os << "NMT";
    case COBType::SYNC         :
        return os << "SYNC";
    case COBType::TIMESTAMP    :
        return os << "TIMESTAMP";
    case COBType::EMERGENCY    :
        return os << "EMERGENCY";
    case COBType::PDO          :
        return os << "PDO";
    case COBType::SDO_TRANSMIT :
        return os << "SDO_TRANSMIT";
    case COBType::SDO_RECEIVE  :
        return os << "SDO_RECEIVE";
    case COBType::ERROR_CONTROL:
        return os << "ERROR_CONTROL";
    };

    return os << "UNKNOWN VALUE : " << val << " - FIX ME - CHECK WARNINGS";
}

bool COBId::operator==(const COBId& r) const
{
    return nodeId == r.nodeId && cobTypeValue == r.cobTypeValue;
}

bool COBId::operator!=(const COBId& r) const
{
    return !(*this == r);
}

bool COBId::operator<(const COBId& r) const // TODO: define a policy
{
    return cobTypeValue < r.cobTypeValue || (cobTypeValue == r.cobTypeValue && nodeId < r.nodeId);
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
    return out << "SDOIndex{index=" << std::hex << sdoIndex.index
      << ", sub-index=" << std::hex << sdoIndex.subIndex << "}";
}

std::string sdoDataToHexString(const SdoData& data) {
    std::ostringstream os;
    os << "<";
    if(data.size() > 0) {
      for(std::size_t i=0; i < data.size()-1; i++) {
          os << hexUppercase << static_cast<uint32_t>(data[i]) << ":";
      }
      os << hexUppercase << static_cast<uint32_t>(data[data.size()-1]);
    }
    os << ">";
    return os.str();
}

}


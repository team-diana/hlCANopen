// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_TYPES_HPP
#define OCO_TYPES_HPP

#include <cstdint>
#include <vector>
#include <ostream>
#include <functional>
#include <tuple>

namespace hlcanopen {

  typedef std::vector<uint8_t> SdoData;

  typedef unsigned int NodeId;
  enum class COBType {
    NMT, SYNC, TIMESTAMP, EMERGENCY, PDO, SDO_TRANSMIT, SDO_RECEIVE, ERROR_CONTROL
  };
  enum COBTypeUniqueCode {
    SDO_TRANSMIT_UNIQUE_CODE = 0b1011,
    SDO_RECEIVE_UNIQUE_CODE = 0b1100
  };

  typedef int PDOIndex;

  class CycleNumber{
    int cycle;
  };

  struct COBId {
  public:
    COBId();
    COBId(NodeId nodeId, unsigned int cobTypeValue);
    NodeId  getNodeId() const;
    unsigned int getCobTypeValue() const;
    unsigned int getCobIdValue() const;
    COBType getCobType() const;

    friend std::ostream& operator<< (std::ostream &out, const COBId& cobId);
    bool operator==(const COBId& r) const;
    bool operator!=(const COBId& r) const;

  private:
    NodeId nodeId;
    unsigned int cobTypeValue;
  };

  struct SDOIndex {
    SDOIndex(unsigned int index, unsigned int subIndex);

    friend std::ostream& operator<< (std::ostream &out, const SDOIndex& sdoIndex);
    bool operator==(const SDOIndex& r) const;
    bool operator!=(const SDOIndex& r) const;

    unsigned int index;
    unsigned int subIndex;
  };

  struct SDOIndexCompare : public std::binary_function<SDOIndex, SDOIndex, bool>
  {
    bool operator()(SDOIndex lhs, SDOIndex rhs) const {
      return std::tie<unsigned int, unsigned int>(lhs.index, lhs.subIndex) <
             std::tie<unsigned int, unsigned int>(rhs.index, rhs.subIndex);
    }
  };

}

#endif // OCO_TYPES_HPP


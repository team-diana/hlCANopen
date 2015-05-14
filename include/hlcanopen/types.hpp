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

  std::ostream& operator<< (std::ostream & os, COBType val);

  enum COBTypeUniqueCode {
    SDO_TRANSMIT_UNIQUE_CODE = 0b1011,
    SDO_RECEIVE_UNIQUE_CODE = 0b1100
  };

  enum NMTMESSAGES {
    NMT_START_REMOTE_NODE = 0x01,
    NMT_STOP_REMOTE_NODE = 0x02,
    NMT_ENTER_PRE_OPERATIONAL = 0x80,
    NMT_RESET_NODE = 0x81,
    NMT_RESET_COMMUNICATION = 0x82
  };


  typedef int PDOIndex;

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
    bool operator<(const COBId& r) const;

  private:
    NodeId nodeId;
    unsigned int cobTypeValue;
  };

  struct SDOIndex {
    SDOIndex(unsigned int index, unsigned int subIndex);
    SDOIndex() {};

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


// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_CAN_MSG_HPP
#define OCO_CAN_MSG_HPP

#include "types.hpp"

namespace hlcanopen {

struct CanMsg {
  COBId cobId;
  uint64_t data = 0;

  // From MSB to LSB
  uint8_t byteat(int index) const {
    return (*this)[index];
  }


  uint8_t& operator[](int index) {
    return byteAtRef(index);
  }

  // From MSB to LSB
  uint8_t operator[](int index) const {
    return byteAtRef(index);
  }

private:
  uint8_t& byteAtRef(int index) const {
    uint8_t* v = reinterpret_cast<uint8_t*>(const_cast<uint64_t*>(&data)); // LSB
    return *(v+7-index);
  }

};

}

#endif // OCO_CAN_MSG_HPP


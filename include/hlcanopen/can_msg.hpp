// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_CAN_MSG_HPP
#define OCO_CAN_MSG_HPP

#include "types.hpp"

#include <sstream>
#include <iomanip>

namespace hlcanopen {

struct CanMsg {
    COBId cobId;
    uint64_t data = 0;

    CanMsg();
    CanMsg(const CanMsg& oth) = default;
    CanMsg(CanMsg&& oth) = default;

    // From MSB to LSB
    uint8_t byteat(int index) const;

    uint8_t& operator[](int index);

    // From MSB to LSB
    uint8_t operator[](int index) const;

    friend std::ostream& operator<<(std::ostream& os, const CanMsg& msg);

    std::string msgDataToStr() const;

private:
    uint8_t& byteAtRef(int index) const;

};

std::ostream& operator<<(std::ostream& os, const CanMsg& msg);


}

#endif // OCO_CAN_MSG_HPP


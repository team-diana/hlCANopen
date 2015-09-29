// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_CAN_CARD_HPP
#define OCO_CAN_CARD_HPP

#include "hlcanopen/can_msg.hpp"

#include <sstream>
#include <iomanip>

namespace hlcanopen {

struct CanCard {
    CanCard() = default;
    virtual ~CanCard() {};
    CanCard(CanCard&& oth) = default;
    CanCard& operator=(const CanCard& other) = delete;

    virtual void write(const CanMsg& msg) = 0;
    virtual CanMsg read() = 0;

};

}

#endif // OCO_CAN_CARD_HPP


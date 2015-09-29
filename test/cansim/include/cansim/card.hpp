// Copyright (C) 2015 team-diana MICanMsg license

#ifndef CARD_HPP
#define CARD_HPP

#include "hlcanopen/can_card.hpp"
#include "hlcanopen/can_msg.hpp"

#include "cansim/bus.hpp"

#include <utility>


class Card : public hlcanopen::CanCard {

public:
    Card(unsigned int id, std::shared_ptr<Bus<hlcanopen::CanMsg>> bus) :
        id(id),
        bus(bus) {
        bus->addCard(*this);
    }

    ~Card() {
        bus->removeCard(*this);
    }

    virtual void write(const hlcanopen::CanMsg& msg) override {
        bus->write(*this, msg);
    }

    virtual hlcanopen::CanMsg read() override {
        return bus->read(*this);
    }

    bool operator==(const Card& rhs) const
    {
        return id == rhs.id;
    }

    bool operator!=(const Card& rhs) const
    {
        return !(*this == rhs);
    }

    unsigned int id;
    std::shared_ptr<Bus<hlcanopen::CanMsg>> bus;
};

#endif // CARD_HPP


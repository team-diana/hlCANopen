// Copyright (C) 2015 team-diana MIT license

#ifndef BUS_LESS_CARD_HPP
#define BUS_LESS_CARD_HPP

#include "cansim/bi_pipe.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/can_card.hpp"
#include "hlcanopen/can_card.hpp"

struct BusLessCard : public hlcanopen::CanCard {
public:
    BusLessCard(std::shared_ptr<BiPipe<hlcanopen::CanMsg>> biPipe) :
        CanCard(),
        biPipe(biPipe) {

    }

    BusLessCard(const BusLessCard& card) : CanCard(), biPipe(card.biPipe) {

    };

    virtual void write(const hlcanopen::CanMsg& canMsg) override {
        biPipe->write(canMsg);
    }

    virtual hlcanopen::CanMsg read() override {
        return biPipe->read();
    }

    static std::tuple<BusLessCard, std::shared_ptr<BiPipe<hlcanopen::CanMsg>>>  makeWithTestBiPipe() {
        auto pipes = BiPipe<hlcanopen::CanMsg>::make();
        std::shared_ptr<BiPipe<hlcanopen::CanMsg>> testPipe = std::get<0>(pipes);
        return std::make_tuple(BusLessCard(std::get<1>(pipes)), testPipe);
    }

private:
    std::shared_ptr<BiPipe<hlcanopen::CanMsg>> biPipe;
};

#endif // BUS_LESS_CARD_HPP


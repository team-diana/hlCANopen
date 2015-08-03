// Copyright (C) 2015 team-diana MIT license

#ifndef CANSIM_BUS_HPP
#define CANSIM_BUS_HPP

#include "cansim/pipe.hpp"

#include <boost/assert.hpp>

#include <utility>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <list>
#include <memory>

template <class U> class Card;

// Non thread-safe message dispatcher
template<typename T> class Bus {
    template<class U> friend class Card;

    struct CardSlot {
        CardSlot(const Card<T>& card, Pipe<T>&& pipe) :
            card(card),
            pipe(std::move(pipe)) {
        }

        const Card<T>& card;
        Pipe<T> pipe;
    };

public:
    Bus() = default;
    Bus(const Bus& bus) = delete;
    void writeEmptyMsg() {
        std::for_each(cards.begin(), cards.end(), [&](CardSlot& c) {
            c.pipe.write(T());
        });
    }

private:
    void addCard(const Card<T>& card) {
        cards.emplace_back(card, Pipe<T>());
    }

    void removeCard(const Card<T>& card) {
        cards.remove_if([&](const CardSlot& c) {
            return c.card == card;
        });
    }

    template<class M> void write(const Card<T>& card, const M& msg) {
        std::for_each(cards.begin(), cards.end(), [&](CardSlot& c) {
            if(c.card != card) c.pipe.write(msg);
        });
    }

    T read(const Card<T>& card) {
        for(auto& c : cards) {
            if(c.card == card) return c.pipe.read();
        }
        BOOST_ASSERT_MSG(false, "non existent card in sim bus.");
        return T();
    }


private:
    std::list<CardSlot> cards;
};


#endif // CANSIM_BUS_HPP


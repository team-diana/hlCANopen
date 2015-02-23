// Copyright (C) 2015 team-diana MIT license

#ifndef CARD_HPP
#define CARD_HPP

#include "cansim/bus.hpp"

#include <utility>


template<class T> class Card {

public:
  Card(unsigned int id, std::shared_ptr<Bus<T>> bus) :
  id(id),
  bus(bus) {
    bus->addCard(*this);
  }

  ~Card() {
    bus->removeCard(*this);
  }

  template<class M> void write(M&& msg) {
    bus->write(*this, std::forward<M>(msg));
  }

  T read() {
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
  std::shared_ptr<Bus<T>> bus;
};

#endif // CARD_HPP


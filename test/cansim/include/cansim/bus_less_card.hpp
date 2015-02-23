// Copyright (C) 2015 team-diana MIT license

#ifndef BUS_LESS_CARD_HPP
#define BUS_LESS_CARD_HPP

#include "cansim/bi_pipe.hpp"

template<typename T> struct BusLessCard  {
public:
  BusLessCard(std::shared_ptr<BiPipe<T>> biPipe) :
  biPipe(biPipe) {

  }
  BusLessCard(const BusLessCard& card) = default;

  void write(const T& canMsg) {
    biPipe->write(canMsg);
  }

  T read() {
    return biPipe->read();
  }

  static std::tuple<BusLessCard, std::shared_ptr<BiPipe<T>>>  makeWithTestBiPipe() {
    auto pipes = BiPipe<T>::make();
    std::shared_ptr<BiPipe<T>> testPipe = std::get<0>(pipes);
    return std::make_tuple(BusLessCard(std::get<1>(pipes)), testPipe);
  }

private:
  std::shared_ptr<BiPipe<T>> biPipe;
};

#endif // BUS_LESS_CARD_HPP


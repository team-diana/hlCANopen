// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE CanMsg_test


#include "hlcanopen/sdo_server_request.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/can_msg_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/bus_less_card.hpp"

#include "boost/test/unit_test.hpp"

#include <sstream>
#include <iostream>

using namespace std;
using namespace hlcanopen;


BOOST_AUTO_TEST_CASE(CanMsgDataStr) {
  CanMsg msg;

  msg[0] = 0x1;
  msg[1] = 0xA;
  msg[2] = 0x10;
  msg[3] = 0xA0;
  msg[4] = 0x11;
  msg[5] = 0xff;

  BOOST_CHECK_EQUAL("<01:0A:10:A0:11:FF:00:00>", msg.msgDataToStr());
}

BOOST_AUTO_TEST_CASE(CanMsgStr) {
  CanMsg msg;

  msg.cobId = COBId(0xA, 0b1111);
  msg[0] = 0xAA;
  msg[1] = 0x0F;

  std::stringstream v;
  v << msg;
  std::string res = v.str();
  BOOST_CHECK_EQUAL("CanMsg[cobId:78A, data:<AA:0F:00:00:00:00:00:00>]", res);
}

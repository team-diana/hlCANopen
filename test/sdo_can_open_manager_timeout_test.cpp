// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE SdoCanOpenManagerTimeout_test

#include "hlcanopen/can_open_manager.hpp"
#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include "test_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/card.hpp"
#include "cansim/bus.hpp"

#include <boost/test/unit_test.hpp>

#include <thread>
#include <iostream>

using namespace hlcanopen;
using namespace std;

typedef Bus<CanMsg> TestBus;
typedef Card<CanMsg> TestCard;

BOOST_AUTO_TEST_CASE(SdoCanOpenManagerTimeoutRemoteRead) {
  std::shared_ptr<TestBus> bus = std::make_shared<TestBus>();
  TestCard cardA(1, bus);
  TestCard cardB(2, bus);

  NodeId nodeA = 1;
  SDOIndex sdoIndex1(0xABCD, 1);
  SDOIndex sdoIndex2(0xDDEE, 0);


  CanOpenManager<TestCard> managerB(cardB, std::chrono::milliseconds(0));
  managerB.initNode(nodeA, NodeManagerType::CLIENT);
  auto result1 = managerB.readSdoRemote<int32_t>(nodeA, sdoIndex1, 1000);
  auto result2 = managerB.readSdoRemote<string>(nodeA, sdoIndex2, 1000);

  thread bt = thread([&](){
    managerB.run();
  });

  bus->writeEmptyMsg();

  auto sdoResult1 = result1.get();
  auto sdoResult2 = result2.get();

  sdoResult1.get();
  sdoResult2.get();

  SdoError error1 = SdoError(SdoErrorCode::TIMEOUT);
  SdoError error2 = SdoError(SdoErrorCode::TIMEOUT);

  BOOST_CHECK_EQUAL(sdoResult1.ok(), false);
  BOOST_CHECK_EQUAL(sdoResult2.ok(), false);

  BOOST_CHECK_EQUAL(sdoResult1.getError().string(), error1.string());
  BOOST_CHECK_EQUAL(sdoResult2.getError().string(), error2.string());

  managerB.stop();

  bus->writeEmptyMsg();

  bt.join();
}

BOOST_AUTO_TEST_CASE(SdoCanOpenManagerRemoteWrite) {
  std::shared_ptr<TestBus> bus = std::make_shared<TestBus>();
  TestCard cardA(1, bus);
  TestCard cardB(2, bus);

  NodeId nodeA = 1;
  SDOIndex sdoIndex1(0xABCD, 1);
  SDOIndex sdoIndex2(0xDDEE, 0);

  int32_t value=0xAABBCCDD;
  string str = "hello world!";

  CanOpenManager<TestCard> managerB(cardB, std::chrono::milliseconds(0));

  thread bt = thread([&](){
    managerB.run();
  });

  managerB.initNode(nodeA, NodeManagerType::CLIENT);
  auto result1 = managerB.writeSdoRemote<int32_t>(nodeA, sdoIndex1, value, 1500);

  SdoError error1 = SdoError(SdoErrorCode::TIMEOUT);

  auto sdoResult1 = result1.get();

  BOOST_CHECK_EQUAL(sdoResult1.ok(), false);
  BOOST_CHECK_EQUAL(sdoResult1.getError().string(), error1.string());

  volatile bool valueReceived = false;
  managerB.writeSdoRemote<string>(nodeA, sdoIndex2, str, [&](SdoResponse<bool> res){
    BOOST_CHECK_EQUAL(res.ok(), false);
    valueReceived = true;
  }, 1000);

  while(!valueReceived ) {}

  managerB.stop();

  bus->writeEmptyMsg();

  bt.join();
#if 0
#endif
}

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
using namespace folly;

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

  result1.wait();
  result2.wait();

  SdoError error1 = SdoError(SdoErrorCode::TIMEOUT);
  SdoError error2 = SdoError(SdoErrorCode::TIMEOUT);


  BOOST_CHECK_EQUAL(result1.hasValue(), false);
  BOOST_CHECK_EQUAL(result2.hasValue(), false);

  BOOST_CHECK_EQUAL(getSdoError(result1).what(), error1.what());
  BOOST_CHECK_EQUAL(getSdoError(result2).what(), error2.what());

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

  result1.wait();

  BOOST_CHECK_EQUAL(result1.hasValue(), false);
  BOOST_CHECK_EQUAL(getSdoError(result1).what(), error1.what());

  volatile bool valueReceived = false;
  managerB.writeSdoRemote<string>(nodeA, sdoIndex2, str, [&](Try<Unit> res){
    BOOST_CHECK_EQUAL(res.hasValue(), false);
    valueReceived = true;
  }, 1000);

  while(!valueReceived ) {}

  managerB.stop();

  bus->writeEmptyMsg();

  bt.join();
#if 0
#endif
}

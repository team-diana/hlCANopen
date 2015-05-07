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

  CanOpenManager<TestCard> managerA(cardA, std::chrono::milliseconds(0));
  NodeId nodeA = 1;
  managerA.initNode(nodeA, NodeManagerType::SERVER);
  SDOIndex sdoIndex1(0xABCD, 1);
  SDOIndex sdoIndex2(0xDDEE, 0);

  int32_t value=0xAABBCCDD;
  managerA.writeSdoLocal(nodeA, sdoIndex1, value);
  string str = "hello world!";
  managerA.writeSdoLocal(nodeA, sdoIndex2, str);

  thread at = thread([&](){
    managerA.run();
  });


  CanOpenManager<TestCard> managerB(cardB, std::chrono::milliseconds(0));
  managerB.initNode(nodeA, NodeManagerType::CLIENT);
  auto result1 = managerB.readSdoRemote<int32_t>(nodeA, sdoIndex1);
  auto result2 = managerB.readSdoRemote<string>(nodeA, sdoIndex2);

  thread bt = thread([&](){
    managerB.run();
  });


  bus->writeEmptyMsg();

  int32_t readValue = result1.get().get();
  string readStr = result2.get().get();
  
  SdoError error1 = SdoError(TIMEOUT);
  SdoError error2 = SdoError(TIMEOUT);

  BOOST_CHECK_EQUAL(result1.ok(), false);
  BOOST_CHECK_EQUAL(result2.ok(), false);

  BOOST_CHECK_EQUAL(result1.get().getError(), error1);
  BOOST_CHECK_EQUAL(result2.get().getError(), error2);
  
  managerA.stop();
  managerB.stop();

  bus->writeEmptyMsg();

  at.join();
  bt.join();
}

BOOST_AUTO_TEST_CASE(SdoCanOpenManagerRemoteWrite) {
  std::shared_ptr<TestBus> bus = std::make_shared<TestBus>();
  TestCard cardA(1, bus);
  TestCard cardB(2, bus);

  CanOpenManager<TestCard> managerA(cardA, std::chrono::milliseconds(0));
  NodeId nodeA = 1;
  managerA.initNode(nodeA, NodeManagerType::SERVER);
  SDOIndex sdoIndex1(0xABCD, 1);
  SDOIndex sdoIndex2(0xDDEE, 0);

  managerA.writeSdoLocal<int32_t>(nodeA, sdoIndex1, 0);
  managerA.writeSdoLocal(nodeA, sdoIndex2, "");

  managerA.setSdoAccessLocal(nodeA, sdoIndex1, EntryAccess::READWRITE);
  managerA.setSdoAccessLocal(nodeA, sdoIndex2, EntryAccess::READWRITE);

  thread at = thread([&](){
    managerA.run();
  });

  int32_t value=0xAABBCCDD;
  string str = "hello world!";

  CanOpenManager<TestCard> managerB(cardB, std::chrono::milliseconds(0));

  thread bt = thread([&](){
    managerB.run();
  });

  managerB.initNode(nodeA, NodeManagerType::CLIENT);
  auto result1 = managerB.writeSdoRemote<int32_t>(nodeA, sdoIndex1, value);

  SdoError error1 = SdoError(TIMEOUT);

  BOOST_CHECK_EQUAL(result1.ok(), false);

  BOOST_CHECK_EQUAL(result1.get().getError(), error1);
  
  volatile bool valueReceived = false;
  managerB.writeSdoRemote<string>(nodeA, sdoIndex2, str, [&](SdoResponse<bool> res){
    BOOST_CHECK_EQUAL(res.ok(), false);
    BOOST_CHECK_EQUAL(result1.getError(), error1);
    valueReceived = true;
  });

  while(!valueReceived ) {}
  managerA.stop();
  managerB.stop();

  // TODO: test may fail here when value is not yet completely transmitted
  int32_t writtenValue = managerA.template readSdoLocal<int32_t>(nodeA, sdoIndex1);
  BOOST_CHECK_EQUAL(value, writtenValue);
  std::string writtenStr = managerA.template readSdoLocal<std::string>(nodeA, sdoIndex2);
  BOOST_CHECK_EQUAL(str, writtenStr);

  bus->writeEmptyMsg();

  at.join();
  bt.join();
}

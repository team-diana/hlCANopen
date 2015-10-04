// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE SdoCanOpenManager_test

#include "hlcanopen/can_open_manager.hpp"
#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/executor/unique_thread_executor.hpp"

#include "test_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/card.hpp"
#include "cansim/bus.hpp"

#include <boost/test/unit_test.hpp>

#include <thread>
#include <functional>
#include <iostream>

using namespace hlcanopen;
using namespace std;
using namespace folly;

typedef Bus<CanMsg> TestBus;
typedef Card TestCard;

BOOST_FIXTURE_TEST_SUITE(SdoCanOpenManagerSuite, TestFixture)

BOOST_AUTO_TEST_CASE(SdoCanOpenManagerRemoteRead) {
    std::shared_ptr<TestBus> bus = std::make_shared<TestBus>();
    TestCard cardA(1, bus);
    TestCard cardB(2, bus);

    CanOpenManager managerA(cardA, std::chrono::milliseconds(0));
    NodeId nodeA = 1;
    managerA.initNode(nodeA, NodeManagerType::SERVER);
    SDOIndex sdoIndex1(0xABCD, 1);
    SDOIndex sdoIndex2(0xDDEE, 0);

    int32_t value=0xAABBCCDD;
    managerA.writeSdoLocal(nodeA, sdoIndex1, value);
    string str = "hello world!";
    managerA.writeSdoLocal(nodeA, sdoIndex2, str);

    thread at = thread([&]() {
        managerA.run();
    });


    CanOpenManager managerB(cardB, std::chrono::milliseconds(0));
    managerB.initNode(nodeA, NodeManagerType::CLIENT);
    auto result1 = managerB.readSdoRemote<int32_t>(nodeA, sdoIndex1);
    auto result2 = managerB.readSdoRemote<string>(nodeA, sdoIndex2);

    thread bt = thread([&]() {
        managerB.run();
    });


    bus->writeEmptyMsg();

    int32_t readValue = result1.get();
    string readStr = result2.get();

    BOOST_CHECK_EQUAL(value, readValue);
    BOOST_CHECK_EQUAL(str, readStr);

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

    CanOpenManager managerA(cardA, std::chrono::milliseconds(0));
    NodeId nodeA = 1;
    managerA.initNode(nodeA, NodeManagerType::SERVER);
    SDOIndex sdoIndex1(0xABCD, 1);
    SDOIndex sdoIndex2(0xDDEE, 0);

    managerA.template writeSdoLocal<int32_t>(nodeA, sdoIndex1, 0);
    managerA.writeSdoLocal(nodeA, sdoIndex2, "");

    managerA.setSdoAccessLocal(nodeA, sdoIndex1, EntryAccess::READWRITE);
    managerA.setSdoAccessLocal(nodeA, sdoIndex2, EntryAccess::READWRITE);

    thread at = thread([&]() {
        managerA.run();
    });

    int32_t value=0xAABBCCDD;
    string str = "hello world!";

    CanOpenManager managerB(cardB, std::chrono::milliseconds(0));

    thread bt = thread([&]() {
        managerB.run();
    });

    managerB.initNode(nodeA, NodeManagerType::CLIENT);
    auto result1 = managerB.writeSdoRemote<int32_t>(nodeA, sdoIndex1, value);

    result1.wait();
    BOOST_CHECK_EQUAL(true, result1.hasValue());


    volatile bool valueReceived = false;
    managerB.writeSdoRemote<string>(nodeA, sdoIndex2, str, [&](Try<Unit> res) {
        BOOST_CHECK_EQUAL(true, res.hasValue());
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

BOOST_AUTO_TEST_CASE(SdoCanOpenManagerUniqueThreadExecutor) {
    std::shared_ptr<TestBus> bus = std::make_shared<TestBus>();
    TestCard cardA(1, bus);
    TestCard cardB(2, bus);

    CanOpenManager managerA(cardA, std::chrono::milliseconds(0));
    NodeId nodeA = 1;
    managerA.initNode(nodeA, NodeManagerType::SERVER);
    SDOIndex sdoIndex1(0xABCD, 1);

    int32_t value=0xAABBCCDD;

    auto uniqueThreadExecutor = std::make_shared<UniqueThreadExecutor>();

    managerA.setupLogging();

    managerA.setDefaultFutureExecutor(uniqueThreadExecutor);
    managerA.writeSdoLocal(nodeA, sdoIndex1, value);

    thread at = thread([&]() {
        managerA.run();
    });

    CanOpenManager managerB(cardB, std::chrono::milliseconds(0));
    managerB.initNode(nodeA, NodeManagerType::CLIENT);
    auto result1 = managerB.readSdoRemote<int32_t>(nodeA, sdoIndex1);

    thread bt = thread([&]() {
        managerB.run();
    });


    bus->writeEmptyMsg();

    int32_t readValue = result1.get();

    BOOST_CHECK_EQUAL(value, readValue);

    managerA.stop();
    managerB.stop();

    bus->writeEmptyMsg();

    at.join();
    bt.join();
}

BOOST_AUTO_TEST_CASE(SdoCanOpenManagerRecursiveLambda) {
    std::shared_ptr<TestBus> bus = std::make_shared<TestBus>();
    TestCard cardA(1, bus);
    TestCard cardB(2, bus);

    CanOpenManager managerA(cardA, std::chrono::milliseconds(0));
    NodeId nodeA = 1;
    managerA.initNode(nodeA, NodeManagerType::SERVER);
    SDOIndex sdoIndex1(0xABCD, 1);

    managerA.writeSdoLocal(nodeA, sdoIndex1, 0);

    thread at = thread([&]() {
        managerA.run();
    });

    CanOpenManager managerB(cardB, std::chrono::milliseconds(0));
    managerB.initNode(nodeA, NodeManagerType::CLIENT);
    auto executor = std::make_shared<UniqueThreadExecutor>();
    managerB.setDefaultFutureExecutor(executor);

    std::atomic<int> counter(0);
    std::function<folly::Future<bool>(int32_t)> recursiveLambda = [&](int32_t value) {
      counter++;
      if(value == 1) {
        Promise<bool> p;
        p.setValue(true);
        return p.getFuture();
      }
      return managerB.template readSdoRemote<int32_t>(nodeA, sdoIndex1).then(recursiveLambda);
    };

    auto result1 = managerB.template readSdoRemote<int32_t>(nodeA, sdoIndex1).then([&](){
      return recursiveLambda(0);
    });

    thread bt = thread([&]() {
        managerB.run();
    });

    bus->writeEmptyMsg();

    while(counter < 10) {
      // no-op;
    }
    managerA.writeSdoLocal(nodeA, sdoIndex1, 1);
    cout << "wrote new value" << endl;

    result1.get();

    managerA.stop();
    managerB.stop();

    bus->writeEmptyMsg();

    at.join();
    bt.join();
}

BOOST_AUTO_TEST_SUITE_END()

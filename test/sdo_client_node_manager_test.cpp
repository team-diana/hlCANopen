// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE SdoClientNodeManager_test

#include "hlcanopen/sdo_client_node_manager.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/bus_less_card.hpp"

#include <folly/futures/Future.h>

#include "boost/test/unit_test.hpp"

#include <iostream>
#include <thread>


#include "hlcanopen/logging/easylogging++.h"

using namespace std;
using namespace hlcanopen;
using namespace folly;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TransStatus);

typedef BusLessCard<CanMsg> TestCard;

BOOST_AUTO_TEST_CASE(SdoClientNodeManagerReadFutureTest) {
    auto testCardAndPipe = TestCard::makeWithTestBiPipe();
    TestCard card(std::get<0>(testCardAndPipe));
    std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
    ObjectDictionary objDict;

    SdoClientNodeManager<TestCard> nodeManager(1, card, objDict);

    SDOIndex sdoIndex(0xAABB, 0);

    int32_t valueToBeRead=0xAABBCCDD;

    auto response = nodeManager.readSdo<int32_t>(sdoIndex);

    std::thread managerThread(
    [&]() {
        CanMsg msg = card.read();
        nodeManager.handleSdoTransmit(msg);
    }
    );

    std::thread otherDeviceThread(
    [&]() {
        CanMsg readResponse;
        readResponse.cobId = COBId(1, SDO_TRANSMIT_UNIQUE_CODE);
        setSdoServerCommandSpecifier(readResponse, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
        setSdoIndex(readResponse, sdoIndex);
        setSdoExpeditedTransfer(readResponse, true);
        setSdoNumberOfBytesOfDataInMsgSdoServer(readResponse, 4, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
        setSdoSizeBit(readResponse, true);
        setLast4Byte(readResponse, valueToBeRead);
        testPipe->write(readResponse);
    }
    );

    response.wait();
    Try<int32_t> sdoResponse = response.getTry();
    managerThread.join();
    otherDeviceThread.join();


    BOOST_CHECK_EQUAL(true, sdoResponse.hasValue());
    BOOST_CHECK_EQUAL(valueToBeRead, sdoResponse.value());
}

BOOST_AUTO_TEST_CASE(SdoClientNodeManagerReadCallbackTest) {
    auto testCardAndPipe = TestCard::makeWithTestBiPipe();
    TestCard card(std::get<0>(testCardAndPipe));
    std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
    ObjectDictionary objDict;

    SdoClientNodeManager<TestCard> nodeManager(1, card, objDict);
    bool stopThread = false;

    SDOIndex sdoIndex(0xAABB, 1);
    uint32_t valueToBeRead=0xCAFECAFE;

    nodeManager.readSdo<int32_t>(sdoIndex, [&](Try<int32_t> sdoResponse) {
        BOOST_CHECK_EQUAL(true, sdoResponse.hasValue());
        BOOST_CHECK_EQUAL(valueToBeRead, sdoResponse.value());
        stopThread = true;
    });

    std::thread managerThread(
    [&]() {
        while(!stopThread) {
            CanMsg msg = card.read();
            nodeManager.handleSdoTransmit(msg);
        }
    }
    );

    std::thread otherDeviceThread(
    [&]() {
        CanMsg readResponse;
        readResponse.cobId = COBId(1, SDO_TRANSMIT_UNIQUE_CODE);
        setSdoIndex(readResponse, sdoIndex);
        setSdoServerCommandSpecifier(readResponse, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
        setSdoNumberOfBytesOfDataInMsgSdoServer(readResponse, 4, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
        setSdoSizeBit(readResponse, true);
        setSdoExpeditedTransfer(readResponse, true);
        setLast4Byte(readResponse, valueToBeRead);
        testPipe->write(readResponse);
    }
    );

    managerThread.join();
    otherDeviceThread.join();
}


BOOST_AUTO_TEST_CASE(SdoClientNodeManagerWriteFutureTest) {
    auto testCardAndPipe = TestCard::makeWithTestBiPipe();
    TestCard card(std::get<0>(testCardAndPipe));
    std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
    ObjectDictionary objDict;

    SdoClientNodeManager<TestCard> nodeManager(1, card, objDict);

    SDOIndex sdoIndex(0xAABB, 0);
    int32_t valueToWrite=0xAABBCCDD;

    auto response = nodeManager.writeSdo(sdoIndex, valueToWrite);

    std::thread managerThread(
    [&]() {
        CanMsg msg = card.read();
        nodeManager.handleSdoTransmit(msg);
    }
    );

    std::thread otherDeviceThread(
    [&]() {
        CanMsg clientWriteReq = testPipe->read();
        int32_t sentData = getLast4Byte(clientWriteReq);
        BOOST_CHECK_EQUAL(valueToWrite, sentData);

        CanMsg writeResponse;
        writeResponse.cobId = COBId(1, SDO_TRANSMIT_UNIQUE_CODE);
        setSdoServerCommandSpecifier(writeResponse, SdoServerCommandSpecifier::INITIATE_DOWNLOAD_RESPONSE);
        setSdoIndex(writeResponse, sdoIndex);
        testPipe->write(writeResponse);
    }
    );

    managerThread.join();
    otherDeviceThread.join();

    response.wait();
    BOOST_CHECK_EQUAL(true, response.hasValue());
}

BOOST_AUTO_TEST_CASE(SdoClientNodeManagerWriteCallbackTest) {
    auto testCardAndPipe = TestCard::makeWithTestBiPipe();
    TestCard card(std::get<0>(testCardAndPipe));
    std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
    ObjectDictionary objDict;

    SdoClientNodeManager<TestCard> nodeManager(1, card, objDict);

    SDOIndex sdoIndex(0xAABB, 0);
    int32_t valueToWrite=0xAABBCCDD;

    nodeManager.writeSdo(sdoIndex, valueToWrite, [&](Try<Unit> result) {
        BOOST_CHECK_EQUAL(true, result.hasValue());
    });

    std::thread managerThread(
    [&]() {
        CanMsg msg = card.read();
        nodeManager.handleSdoTransmit(msg);
    }
    );

    std::thread otherDeviceThread(
    [&]() {
        CanMsg clientWriteReq = testPipe->read();
        int32_t sentData = getLast4Byte(clientWriteReq);
        BOOST_CHECK_EQUAL(valueToWrite, sentData);

        CanMsg writeResponse;
        writeResponse.cobId = COBId(1, SDO_TRANSMIT_UNIQUE_CODE);
        setSdoServerCommandSpecifier(writeResponse, SdoServerCommandSpecifier::INITIATE_DOWNLOAD_RESPONSE);
        setSdoIndex(writeResponse, sdoIndex);
        testPipe->write(writeResponse);
    }
    );

    managerThread.join();
    otherDeviceThread.join();
}


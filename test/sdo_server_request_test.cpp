// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE SdoServerRequest_test
#include <iostream>

#include "hlcanopen/sdo_server_request.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/can_msg_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/bus_less_card.hpp"

#include "boost/test/unit_test.hpp"

using namespace std;
using namespace hlcanopen;


typedef BusLessCard<CanMsg> TestCard;

BOOST_AUTO_TEST_CASE(SdoServerExpeditedReadRequestTest) {
  auto testCardAndPipe = TestCard::makeWithTestBiPipe();
  TestCard card(std::get<0>(testCardAndPipe));
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
  ObjectDictionary objDict;
  SDOIndex readIndex(0xABCD, 1);
  uint32_t valueToBeRead = 0xAABBCCDD;
  objDict.write(readIndex, valueToBeRead);
  NodeId nodeId = 1;

  // setup read request start msg
  CanMsg canMsg;
  canMsg.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
  setSdoClientCommandSpecifier(canMsg, SdoClientCommandSpecifier::INITIATE_UPLOAD);
  setSdoIndex(canMsg, readIndex);

  SdoServerRequest<TestCard> sdoServerRequest(nodeId, card, objDict, canMsg);

  BOOST_CHECK_EQUAL(true, sdoServerRequest.isCompleted());
  CanMsg serverAnswer = testPipe->read();

  uint32_t readValue = convertSdoData<uint32_t>(getBytesAsData(serverAnswer, 4, 7));
  BOOST_CHECK_EQUAL(valueToBeRead, readValue);
}

BOOST_AUTO_TEST_CASE(SdoServerSegmentedReadRequestTest) {
  string stringToRead = "hello";
  for(int i = 0; i < 50; i++) {
    auto testCardAndPipe = TestCard::makeWithTestBiPipe();
    TestCard card(std::get<0>(testCardAndPipe));
    std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
    ObjectDictionary objDict;
    SDOIndex readIndex(0xABCD, 1);
    string valueToBeRead = stringToRead;
    objDict.write(readIndex, valueToBeRead);
    NodeId nodeId = 1;

    // setup read request start msg
    CanMsg canMsg;
    canMsg.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
    setSdoClientCommandSpecifier(canMsg, SdoClientCommandSpecifier::INITIATE_UPLOAD);
    setSdoIndex(canMsg, readIndex);

    SdoServerRequest<TestCard> sdoServerRequest(nodeId, card, objDict, canMsg);

    // Receive start upload ack message
    BOOST_CHECK_EQUAL(false, sdoServerRequest.isCompleted());
    CanMsg startReadAck = testPipe->read();
    BOOST_CHECK_EQUAL(false, sdoExpeditedTransferIsSet(startReadAck));

    SdoData readData;
    bool nextToggleBit = false;
    while(true) {
      CanMsg nextSegmentAsk;
      nextSegmentAsk.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
      setSdoClientCommandSpecifier(nextSegmentAsk, SdoClientCommandSpecifier::UPLOAD_SEGMENT);
      setSdoIndex(nextSegmentAsk, readIndex);
      setSdoToggleBit(nextSegmentAsk, nextToggleBit);
      nextToggleBit = !nextToggleBit;
      sdoServerRequest.newMsg(nextSegmentAsk);

      CanMsg serverAnswer = testPipe->read();
      unsigned int numberOfBytesSent = getSdoNumberOfBytesOfDataInMsg(serverAnswer);
      SdoData sentData = getBytesAsData(serverAnswer, 1, numberOfBytesSent);
      readData.insert(readData.end(), sentData.begin(), sentData.end());
      if(sdoNoMoreSegmentBitIsSet(serverAnswer))
        break;
    }

    BOOST_CHECK_EQUAL(true, sdoServerRequest.isCompleted());

    string readValue = convertSdoData<string>(readData);
    BOOST_CHECK_EQUAL(valueToBeRead, readValue);

    stringToRead += ('a'+(i%25));
  }
}

BOOST_AUTO_TEST_CASE(SdoServerExpeditedWriteRequestTest) {
  auto testCardAndPipe = TestCard::makeWithTestBiPipe();
  TestCard card(std::get<0>(testCardAndPipe));
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);

  ObjectDictionary objDict;
  SDOIndex writeIndex(0xABCD, 1);
  objDict.write(writeIndex, 0u);
  objDict.setAccess(writeIndex, EntryAccess::READWRITE);
  NodeId nodeId = 1;

  // setup write expedited request
  uint32_t valueToWrite = 0xAABBCCDDu;
  CanMsg canMsg;
  canMsg.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
  setSdoClientCommandSpecifier(canMsg, SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
  setSdoIndex(canMsg, writeIndex);
  setSdoExpeditedTransfer(canMsg, true);
  setLast4Byte(canMsg, valueToWrite);

  SdoServerRequest<TestCard> sdoServerRequest(nodeId, card, objDict, canMsg);

  BOOST_CHECK_EQUAL(true, sdoServerRequest.isCompleted());
  ODEntryValue writtenValue = objDict.read(writeIndex);

  BOOST_CHECK_EQUAL(valueToWrite, boost::get<uint32_t>(writtenValue));
}

BOOST_AUTO_TEST_CASE(SdoServerSegmentedWriteRequestTest) {
  string stringToWrite = "hello";
  for(int testIter = 0; testIter < 50; testIter++) {
    auto testCardAndPipe = TestCard::makeWithTestBiPipe();
    TestCard card(std::get<0>(testCardAndPipe));
    std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<1>(testCardAndPipe);
    ObjectDictionary objDict;
    SDOIndex writeIndex(0xABCD, 1);
    objDict.write(writeIndex, "previous");
    objDict.setAccess(writeIndex, EntryAccess::READWRITE);
    NodeId nodeId = 1;

    // setup read request start msg
    CanMsg canMsg;
    canMsg.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
    setSdoClientCommandSpecifier(canMsg, SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
    setSdoExpeditedTransfer(canMsg, false);
    setSdoIndex(canMsg, writeIndex);

    SdoServerRequest<TestCard> sdoServerRequest(nodeId, card, objDict, canMsg);

    // Receive start upload ack message
    BOOST_CHECK_EQUAL(false, sdoServerRequest.isCompleted());
    CanMsg startWriteAck = testPipe->read();
    BOOST_CHECK_EQUAL(false, sdoExpeditedTransferIsSet(startWriteAck));

    bool nextToggleBit = false;
    unsigned int currentByteToSend = 0;
    SdoData writeData = convertValue(stringToWrite);
    bool completed = false;
    while(!completed) {
      CanMsg writeSegment;
      writeSegment.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
      setSdoClientCommandSpecifier(writeSegment, SdoClientCommandSpecifier::DOWNLOAD_SEGMENT);
      setSdoIndex(writeSegment, writeIndex);
      setSdoToggleBit(writeSegment, nextToggleBit);
      nextToggleBit = !nextToggleBit;
      int i;
      for(i = 0; i < 7 && currentByteToSend < writeData.size(); i++) {
        writeSegment[1+i] = writeData[currentByteToSend++];
      }
      setSdoNumberOfBytesOfDataInMsgSdoClient(writeSegment, i, SdoClientCommandSpecifier::DOWNLOAD_SEGMENT);
      if(currentByteToSend >= writeData.size()) {
        completed = true;
      }
      setSdoNoMoreSegmentBit(writeSegment, completed);

      sdoServerRequest.newMsg(writeSegment);

      testPipe->read();
    }

    BOOST_CHECK_EQUAL(true, sdoServerRequest.isCompleted());

    string writtenValue = boost::get<string>(objDict.read(writeIndex));
    BOOST_CHECK_EQUAL(stringToWrite, writtenValue);

    stringToWrite += ('a'+(testIter%25));
  }
}


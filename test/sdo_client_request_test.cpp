// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE SdoClientRequest_test
#include <iostream>

#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"

#include "test_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/bus_less_card.hpp"

#include "boost/test/unit_test.hpp"

#include "hlcanopen/logging/easylogging++.h"

using namespace std;
using namespace hlcanopen;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TransStatus);

typedef BusLessCard<CanMsg> TestCard;

BOOST_AUTO_TEST_CASE(SdoClientExpeditedReadTest) {
  auto pipes = BiPipe<CanMsg>::make();
  TestCard card(std::get<1>(pipes));
  NodeId nodeId = 1;
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<0>(pipes);

  SdoClient<TestCard> client(nodeId, card);

  BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
  BOOST_CHECK_EQUAL(true, client.getSdoError().is_no_error());

  SDOIndex sdoIndex(0xABCD, 1);

  client.readFromNode(sdoIndex);

  CanMsg sdoInitReadMsg = testPipe->read();
  BOOST_CHECK_EQUAL(COBId(nodeId, SDO_RECEIVE_COB_ID), sdoInitReadMsg.cobId);
  BOOST_CHECK_EQUAL(sdoIndex, getSdoIndex(sdoInitReadMsg));
  BOOST_CHECK_EQUAL(TransStatus::CONT, client.getTransStatus());

  CanMsg readResponse;
  readResponse.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_TRANSMIT_UNIQUE_CODE);
  setSdoServerCommandSpecifier(readResponse, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
  setSdoIndex(readResponse, sdoIndex);
  setLast4Byte(readResponse, 0xAABBCCDD);
  setSdoExpeditedTransfer(readResponse, true);
  setSdoNumberOfBytesOfDataInMsgSdoServer(readResponse, 4, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
  client.newMsg(readResponse);

  BOOST_CHECK_EQUAL(TransStatus::END_OK, client.getTransStatus());
  BOOST_CHECK_EQUAL(0xAABBCCDD, convertSdoData<uint32_t>(client.getResponseData()));
  client.cleanForNextRequest();
  BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
}


BOOST_AUTO_TEST_CASE(SdoClientSegmentedReadTest) {
  auto pipes = BiPipe<CanMsg>::make();
  TestCard card(std::get<1>(pipes));
  NodeId nodeId = 1;
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<0>(pipes);

  SdoClient<TestCard> client(nodeId, card);

  for(unsigned int testIter = 0; testIter < 50; testIter++) {
    BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
    BOOST_CHECK_EQUAL(true, client.getSdoError().is_no_error());

    string valueToRead = generateString(5+testIter);
    SDOIndex sdoIndex(0xABCD, 1);

    client.readFromNode(sdoIndex);

    CanMsg sdoInitReadMsg = testPipe->read();
    BOOST_CHECK_EQUAL(COBId(nodeId, SDO_RECEIVE_COB_ID), sdoInitReadMsg.cobId);
    BOOST_CHECK_EQUAL(sdoIndex, getSdoIndex(sdoInitReadMsg));
    BOOST_CHECK_EQUAL(TransStatus::CONT, client.getTransStatus());

    CanMsg readResponse;
    readResponse.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_TRANSMIT_UNIQUE_CODE);
    setSdoServerCommandSpecifier(readResponse, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
    setSdoIndex(readResponse, sdoIndex);
    setSdoExpeditedTransfer(readResponse, false);
    client.newMsg(readResponse);

    SdoData data = convertValue(valueToRead);
    bool completed = false;
    unsigned int currentByteToSend = 0;
    bool nextToggleBit = false;
    while(!completed) {
      testPipe->read();
      BOOST_CHECK_EQUAL(TransStatus::CONT, client.getTransStatus());

      CanMsg serverSegment;
      serverSegment.cobId = COBId(nodeId, SDO_TRANSMIT_UNIQUE_CODE);
      setSdoServerCommandSpecifier(serverSegment, SdoServerCommandSpecifier::UPLOAD_SEGMENT_RESPONSE);
      setSdoToggleBit(serverSegment, nextToggleBit);
      nextToggleBit = !nextToggleBit;
      int i;
      for(i = 0; i < 7 && currentByteToSend < data.size(); i++) {
        serverSegment[1+i] = data[currentByteToSend++];
      }
      setSdoNumberOfBytesOfDataInMsgSdoServer(serverSegment, i, SdoServerCommandSpecifier::UPLOAD_SEGMENT_RESPONSE);
      if(currentByteToSend == data.size()) {
        completed = true;
      }
      setSdoNoMoreSegmentBit(serverSegment, completed);
      client.newMsg(serverSegment);
    }

    BOOST_CHECK_EQUAL(TransStatus::END_OK, client.getTransStatus());
    BOOST_CHECK_EQUAL(valueToRead, convertSdoData<string>(client.getResponseData()));
    client.cleanForNextRequest();
    BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
  }
}

BOOST_AUTO_TEST_CASE(SdoClientExpeditedWriteTest) {
  auto pipes = BiPipe<CanMsg>::make();
  TestCard card(std::get<1>(pipes));
  NodeId nodeId = 1;
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<0>(pipes);

  SdoClient<TestCard> client(nodeId, card);

  BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
  BOOST_CHECK_EQUAL(true, client.getSdoError().is_no_error());

  SDOIndex sdoIndex(0xABCD, 1);

  uint32_t valueToWrite = 0xAABBCCDD;
  SdoData dataToWrite = convertValue(valueToWrite);
  client.writeToNode(sdoIndex, dataToWrite);

  CanMsg sdoInitWriteMsg = testPipe->read();
  BOOST_CHECK_EQUAL(COBId(nodeId, SDO_RECEIVE_COB_ID), sdoInitWriteMsg.cobId);
  BOOST_CHECK_EQUAL(sdoIndex, getSdoIndex(sdoInitWriteMsg));
  BOOST_CHECK_EQUAL(TransStatus::CONT, client.getTransStatus());
  SdoData writtenData = getBytesAsData(sdoInitWriteMsg, 4, 7);
  uint32_t writtenValue = convertSdoData<uint32_t>(writtenData);

  CanMsg writeResponse;
  writeResponse.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_TRANSMIT_UNIQUE_CODE);
  setSdoServerCommandSpecifier(writeResponse, SdoServerCommandSpecifier::INITIATE_DOWNLOAD_RESPONSE);
  setSdoIndex(writeResponse, sdoIndex);
  client.newMsg(writeResponse);

  BOOST_CHECK_EQUAL(TransStatus::END_OK, client.getTransStatus());
  BOOST_CHECK_EQUAL(valueToWrite, writtenValue);
  client.cleanForNextRequest();
  BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
}

BOOST_AUTO_TEST_CASE(SdoClientSegmentedWriteTest) {
  auto pipes = BiPipe<CanMsg>::make();
  TestCard card(std::get<1>(pipes));
  NodeId nodeId = 1;
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<0>(pipes);

  SdoClient<TestCard> client(nodeId, card);

  BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
  BOOST_CHECK_EQUAL(true, client.getSdoError().is_no_error());

  SDOIndex sdoIndex(0xABCD, 1);

  for(unsigned int testIter = 0; testIter < 50; testIter++) {
    string valueToWrite = generateString(testIter + 5);
    SdoData dataToWrite = convertValue(valueToWrite);
    client.writeToNode(sdoIndex, dataToWrite);

    CanMsg clientStartWriteReq = testPipe->read();
    BOOST_CHECK_EQUAL(COBId(nodeId, SDO_RECEIVE_COB_ID), clientStartWriteReq.cobId);
    BOOST_CHECK_EQUAL(sdoIndex, getSdoIndex(clientStartWriteReq));
    BOOST_CHECK_EQUAL(TransStatus::CONT, client.getTransStatus());

    CanMsg serverStartWriteAns;
    serverStartWriteAns.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_TRANSMIT_UNIQUE_CODE);
    setSdoServerCommandSpecifier(serverStartWriteAns, SdoServerCommandSpecifier::INITIATE_DOWNLOAD_RESPONSE);
    setSdoIndex(serverStartWriteAns, sdoIndex);
    client.newMsg(serverStartWriteAns);

    bool completed = false;
    bool nextToggleBit = false;
    SdoData sentData;
    while(!completed) {
      CanMsg clientWriteSegment = testPipe->read();
      BOOST_CHECK_EQUAL(COBId(nodeId, SDO_RECEIVE_COB_ID), clientWriteSegment.cobId);
      BOOST_CHECK_EQUAL(TransStatus::CONT, client.getTransStatus());
      uint32_t numberOfSentBytes = getSdoNumberOfBytesOfDataInMsg(clientWriteSegment);
      SdoData newData = getBytesAsData(clientWriteSegment, 1, numberOfSentBytes);
      sentData.insert(sentData.end(), newData.begin(), newData.end());
      BOOST_CHECK_EQUAL(nextToggleBit, sdoToggleBitIsSet(clientWriteSegment));
      if(sdoNoMoreSegmentBitIsSet(clientWriteSegment)) {
        completed = true;
      }

      CanMsg writeResponse;
      writeResponse.cobId = COBId(nodeId, COBTypeUniqueCode::SDO_TRANSMIT_UNIQUE_CODE);
      setSdoServerCommandSpecifier(writeResponse, SdoServerCommandSpecifier::DOWNLOAD_SEGMENT_RESPONSE);
      setSdoIndex(writeResponse, sdoIndex);
      setSdoToggleBit(writeResponse, nextToggleBit);
      nextToggleBit = !nextToggleBit;
      client.newMsg(writeResponse);
    }

    string writtenValue = convertSdoData<std::string>(sentData);
    BOOST_CHECK_EQUAL(TransStatus::END_OK, client.getTransStatus());
    BOOST_CHECK_EQUAL(valueToWrite, writtenValue);
    client.cleanForNextRequest();
    BOOST_CHECK_EQUAL(TransStatus::NO_TRANS, client.getTransStatus());
  }
}


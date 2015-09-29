// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE PdoConfiguration_test

#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/pdo_configuration.hpp"
#include "hlcanopen/node_manager.hpp"
#include "hlcanopen/pdo_client.hpp"

#include "hlcanopen/logging/easylogging++.h"

#include "test_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/bus_less_card.hpp"

#include <boost/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/core/ignore_unused.hpp>

#include <iostream>
#include <bitset>
#include <queue>


using namespace std;
using namespace hlcanopen;
using namespace folly;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TransStatus);

typedef BusLessCard TestCard;

void printSdoData(const SdoData& data) {
  std::cout << "SdoData: " << sdoDataToHexString(data) << std::endl;
}

// Emulates a node manager that is able to send sdo messages.
struct NodeManagerMock {
  std::queue<SdoData> dataQueue;

  template<typename T> folly::Future<folly::Unit> writeSdoRemote(const SDOIndex& sdoIndex, T data,
                     long timeout = 5000) {
    boost::ignore_unused(sdoIndex);
    boost::ignore_unused(timeout);
    Promise<folly::Unit> a;
    dataQueue.push(convertValue<T>(data));
    a.setValue();
    return a.getFuture();
  }

  // Pop received message in FIFO order
  SdoData popMessage() {
    BOOST_ASSERT(dataQueue.size() > 0);
    SdoData data = dataQueue.front();
    dataQueue.pop();
    return data;
  }

};


BOOST_AUTO_TEST_CASE(PdoConfigurationTest) {
  auto pipes = BiPipe<CanMsg>::make();
  TestCard card(std::get<1>(pipes));

  NodeManagerMock nodeManagerMock;
  PdoClient<NodeManagerMock> client(nodeManagerMock, card);


  PdoConfiguration config(TPDO, 1);

  COBIdPdoEntry cobId;
  cobId.setCobId(COBId(1, 0x181));
  cobId.enable29bitId(true);
  cobId.enablePdo(true);
  cobId.enableRtr(false);

  config.setCobId(cobId);
  config.setTransmissionType(ASYNCHRONOUS, 1);
  config.setNumberOfEntries();

  config.addMapping(SDOIndex(0x1234, 0x00), SDOIndex(0x2002, 0x00), 0x20);
  config.addMapping(SDOIndex(0x4321, 0x00), SDOIndex(0x8765, 0x00), 0x10);

  client.writeConfiguration(config);

  /* 0x01 - Disable the PDO configuration */
  SdoData data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* Disable the PDO mapping */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* 0x02 */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* 0x03 */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* 0x04 */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* 0x05 */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* 1st object mapped */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* 2nd object mapped */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* Enable PDO mapping */
  data = nodeManagerMock.popMessage();
  printSdoData(data);


  /* Enable PDO */
  data = nodeManagerMock.popMessage();
  printSdoData(data);

}

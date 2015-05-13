// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE PdoClient_test
#include <iostream>
#include <bitset>

#include "hlcanopen/sdo_client.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/pdo_configuration.hpp"
#include "hlcanopen/pdo_client.hpp"
#include "hlcanopen/node_manager.hpp"

#include "hlcanopen/logging/easylogging++.h"


#include "test_utils.hpp"

#include "cansim/bi_pipe.hpp"
#include "cansim/bus_less_card.hpp"

#include "boost/test/unit_test.hpp"


using namespace std;
using namespace hlcanopen;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TransStatus);

void print_bytes(CanMsg msg)
{
  size_t i;

  printf("[ ");
  for(i = 0; i < 8; i++)
  {
    printf("%02x ", msg[i]);
  }
  printf("]\n");
}

typedef BusLessCard<CanMsg> TestCard;

BOOST_AUTO_TEST_CASE(PdoClientTest) {
  auto pipes = BiPipe<CanMsg>::make();
  TestCard card(std::get<1>(pipes));
  NodeId nodeId = 1;
  std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<0>(pipes);

  NodeManager<TestCard> manager(nodeId, card, NodeManagerType::CLIENT);
  
  PdoConfiguration config(RPDO, 1);
  
  COBIdPdoEntry cobId;
  cobId.setCobId(COBId(1, 0x181));
  cobId.enable29bitId(true);
  cobId.enablePdo(true);
  cobId.enableRtr(false);
  
  config.setCobId(cobId);
  config.setTransmissionType(ASYNCHRONOUS, 1);
  config.setNumberOfEntries();
  
  config.addMapping(SDOIndex(0x1234, 0x00), SDOIndex(0x2002, 0x00), 0x40);
//  config.addMapping(SDOIndex(0x4321, 0x00), SDOIndex(0x8765, 0x00), 0x40);
  
  manager.writePdoConfiguration(config);
  
  manager.writeSdoLocal<int>(SDOIndex(0x1234, 0), 1);
  
  manager.writeSdoRemote<int>(SDOIndex(0x1234, 0), 1);

  /* 0x01 - Disable the PDO configuration */
  CanMsg msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* Disable the PDO mapping */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* 0x02 */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* 0x03 */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* 0x04 */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* 0x05 */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* 1st object mapped */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* 2nd object mapped */
//  msg = testPipe->read();
//  print_bytes(msg);
//  printf("\n");
  
  /* Enable PDO mapping */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
  
  /* Enable PDO */
  msg = testPipe->read();
  print_bytes(msg);
  printf("\n");
}

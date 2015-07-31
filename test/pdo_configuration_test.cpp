// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE PdoConfiguration_test
#include <iostream>
#include <bitset>

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

#include "boost/test/unit_test.hpp"


using namespace std;
using namespace hlcanopen;
using namespace folly;

BOOST_TEST_DONT_PRINT_LOG_VALUE(TransStatus);

typedef BusLessCard<CanMsg> TestCard;

void print_bytes(const CanMsg& msg) {
  std::cout << msg << std::endl;
}

template <typename C> class NodeManagerMock {
  template<typename T> folly::Future<folly::Unit> writeSdoRemote(const SDOIndex& sdoIndex, T data,
                     long timeout = 5000) {
    Promise<folly::Unit> a;
    return a.getFuture();
  }
};

BOOST_AUTO_TEST_CASE(PdoConfigurationTest) {
//   auto pipes = BiPipe<CanMsg>::make();
//   TestCard card(std::get<1>(pipes));
//   NodeId nodeId = 1;
//   std::shared_ptr<BiPipe<CanMsg>> testPipe = std::get<0>(pipes);
//
//   NodeManagerMock<TestCard> nodeManagerMock;
//   PdoClient<NodeManagerMock<TestCard>, TestCard> client(nodeManagerMock, card);
//
//
//   PdoConfiguration config(TPDO, 1);
//
//   COBIdPdoEntry cobId;
//   cobId.setCobId(COBId(1, 0x181));
//   cobId.enable29bitId(true);
//   cobId.enablePdo(true);
//   cobId.enableRtr(false);
//
//   config.setCobId(cobId);
//   config.setTransmissionType(ASYNCHRONOUS, 1);
//   config.setNumberOfEntries();
//
//   config.addMapping(SDOIndex(0x1234, 0x00), SDOIndex(0x2002, 0x00), 0x20);
//   config.addMapping(SDOIndex(0x4321, 0x00), SDOIndex(0x8765, 0x00), 0x10);
//
//   std::thread nodeManagerThread {[](){
//   /* 0x01 - Disable the PDO configuration */
//   CanMsg msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* Disable the PDO mapping */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* 0x02 */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* 0x03 */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* 0x04 */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* 0x05 */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* 1st object mapped */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* 2nd object mapped */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* Enable PDO mapping */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//
//   /* Enable PDO */
//   msg = testPipe->read();
//   print_bytes(msg);
//   printf("\n");
//       nodeManager.newMsg();
//   }};
//
//   client.writeConfiguration(config);

}

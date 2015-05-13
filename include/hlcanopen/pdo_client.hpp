// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_PDO_CLIENT_HPP
#define OCO_PDO_CLIENT_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/sdo_error.hpp"
#include "hlcanopen/sdo_client_node_manager.hpp"
#include "hlcanopen/pdo_configuration.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/can_msg_utils.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/utils.hpp"

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include "logging/easylogging++.h"

namespace hlcanopen {
  
  enum CommunicationParamSubIndex {
    NUMBER_OF_ENTRIES_SUB_INDEX = 0,
    COB_ID_SUB_INDEX = 1,
    TRANSMISSION_TYPE_SUB_INDEX = 2,
    INHIBIT_TIME_SUB_INDEX = 3,
    RESERVED_SUB_INDEX = 4,
    EVENT_TIMER_SUB_INDEX = 5
  };
  
  struct PDOEntry {
    PDOEntry() {};
    uint8_t length; // Useless for RPDO, but we use the same struct
		    // to have a single PDOMap
    SDOIndex local_object;
  };
    

  template<class C> class PdoClient {
    typedef boost::coroutines::asymmetric_coroutine<CanMsg> coroutine;
    
      NodeId nodeId;
      C& card;
      std::shared_ptr<SdoClientNodeManager<C>> sdoManager;
      std::unique_ptr<coroutine::push_type> sdoCoroutine;
      SdoData receivedData;
      TransStatus currentTransStatus;
      SdoError currentSdoError;
      ObjectDictionary od;
      
      std::map<COBId, std::vector<PDOEntry>> PDOMap;

    public:
      PdoClient(NodeId nodeId, C& card, std::shared_ptr<SdoClientNodeManager<C>> sdoM, ObjectDictionary& objDict) :
      nodeId(nodeId),
      card(card),
      sdoManager(sdoM),
      od(objDict)
      {
      }
      
      void writeConfiguration(PdoConfiguration configuration) {
	unsigned int index = configuration.getPdoNumber() - 1;
	const SdoData zero_data{0, 0, 0, 0};
	
	/* Disable the PDO configuration */
	std::future<SdoResponse<bool>> res = sdoManager->writeSdo(SDOIndex(index, COB_ID_SUB_INDEX), 0);
#if 0
	if (!res.get().ok()) {
	  std::cout << "future not ok" << std::endl;
	  return;
	}
#endif

	/* Disable the PDO mapping */
	sdoManager->writeSdo(SDOIndex(index + 0x200, 0), 0);

	sdoManager->writeSdo(SDOIndex(index, TRANSMISSION_TYPE_SUB_INDEX), configuration.getTransmissionTypeValue());

	sdoManager->writeSdo(SDOIndex(index, INHIBIT_TIME_SUB_INDEX), configuration.getInhibitTime());

	sdoManager->writeSdo(SDOIndex(index, RESERVED_SUB_INDEX), configuration.getReserved());

	sdoManager->writeSdo(SDOIndex(index, EVENT_TIMER_SUB_INDEX), configuration.getEventTimer());

	auto mapp = configuration.getMap();
	uint8_t i = 1;
	PDOEntry entry;
	int s = mapp.size() + 1; // index 0 is wasted
	std::vector<PDOEntry> vect(s);
	for (auto it = mapp.begin(); it != mapp.end(); it++, i++) {
	   entry.length = it->length;
	   entry.local_object = it->local;
	   sdoManager->writeSdo(SDOIndex(index + 0x200, i), it->getValue());
	   vect[it->position] = entry;
	}

	PDOMap[configuration.getCobIdPdo()] = vect;

	sdoManager->writeSdo(SDOIndex(index + 0x200, 0), (uint8_t) i - 1);

	/* Enable the PDO */
	sdoManager->writeSdo(SDOIndex(index, COB_ID_SUB_INDEX), configuration.getCobIdValue());
      }
 
      void receiveTPDO(CanMsg& msg) {
	unsigned int pos;
	for (auto entry : PDOMap(msg.cobId)) { // XXX: Is it ordered from index 0 to length()-1?
	  SdoData data = getBytesAsData(msg, pos, pos + entry.second.length - 1);
	  od.write(entry.second.local, /* cast to ODEntry */ data);
	  pos += entry.second.length;
	}
      }

      void writeRPDO(COBId cobId) {
	CanMsg msg;
	msg.cobId = cobId;
	uint64_t d = 0;
	unsigned int pos = 0;
	for (auto entry : PDOMap(msg.cobId)) {
	  pos += entry.second.local_object;
	  msg.data |= od.read(entry.second.local_object) << (64 - pos);
	}

	card.write(msg);
      }
  };

}

#endif // OCO_PDO_CLIENT_HPP


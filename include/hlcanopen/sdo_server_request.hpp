// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_SERVER_REQUEST_HPP
#define OCO_SDO_SERVER_REQUEST_HPP

#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/sdo_error.hpp"
#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/types.hpp"
#include "hlcanopen/can_msg_utils.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/sdo_entry_converter_visitor.hpp"

#include "hlcanopen/logging/easylogging++.h"

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <memory>
#include <utility>

namespace hlcanopen {

typedef boost::coroutines::asymmetric_coroutine<CanMsg> coroutine;

template<class C> class SdoServerRequest {

public:
  SdoServerRequest(NodeId nodeId, C& card, ObjectDictionary& objDict, const CanMsg& startMsg) :
    nodeId(nodeId),
    card(card),
    objDict(objDict),
    completed(false) {
      dispatchStartMsg(startMsg);
    }

  void newMsg(const CanMsg& msg) {
    BOOST_ASSERT_MSG(serverCoroutine != nullptr, "serverCoroutine is null");
    (*serverCoroutine)(msg);
  }

  bool isCompleted() {
    return completed;
  }


private:
  void dispatchStartMsg(CanMsg startMsg) {
    SdoClientCommandSpecifier commandSpec = getSdoClientCommandSpecifier(startMsg);
    if(commandSpec == SdoClientCommandSpecifier::INITIATE_DOWNLOAD) {
      startWrite(startMsg);
    } else if (commandSpec == SdoClientCommandSpecifier::INITIATE_UPLOAD) {
      startRead(startMsg);
    } else {
      LOG(ERROR) << "unexpected sdo client command specifier: " << static_cast<int>(commandSpec);
    }
  }

  void startRead(CanMsg startMsg) {
    SDOIndex sdoIndex = getSdoIndex(startMsg);
    ODEntryValue value = objDict.read(sdoIndex);
    SdoEntryConverterVisitor converterVisitor;
    SdoData sdoData = value.apply_visitor(converterVisitor);
    if(sdoData.size() <= 4)
      doExpeditedRead(sdoIndex, sdoData);
    else
      startSegmentedRead(sdoIndex, sdoData);
  }

  void doExpeditedRead(SDOIndex sdoIndex, const SdoData& sdoData) {
    CanMsg answer;
    answer.cobId = makeAnsCobId();
    setSdoServerCommandSpecifier(answer, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
    setSdoExpeditedTransfer(answer, true);
    setSdoSizeBit(answer, true);
    setSdoNumberOfBytesOfDataInMsgSdoServer(answer, sdoData.size(),
                                            SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
    setSdoIndex(answer, sdoIndex);
    for(unsigned int i=0; i < sdoData.size(); i++) {
      answer[4+i] = sdoData[i];
    }
    card.write(answer);
    completed = true;
  }

  void startSegmentedRead(SDOIndex sdoIndex, const SdoData& sdoData) {
    CanMsg answer;
    answer.cobId = makeAnsCobId();
    setSdoServerCommandSpecifier(answer, SdoServerCommandSpecifier::INITIATE_UPLOAD_RESPONSE);
    setSdoExpeditedTransfer(answer, false);
    setSdoSizeBit(answer, true);
    setLast4Byte(answer, sdoData.size());
    setSdoIndex(answer, sdoIndex);
    card.write(answer);
    startSegmentedReadCoroutine(sdoIndex, sdoData);
  }

  void startSegmentedReadCoroutine(SDOIndex sdoIndex, const SdoData& sdoData) {
    serverCoroutine = std::make_unique<coroutine::push_type>([&, sdoData](coroutine::pull_type& in) {
      unsigned int byteToTransfer = 0;
      bool nextToggleBit = 0;
      const unsigned int dataSize = sdoData.size();
      while(true) {
        // Check request
        CanMsg request = in.get();
        SdoClientCommandSpecifier requestCommandSpec = getSdoClientCommandSpecifier(request);
        if(requestCommandSpec == SdoClientCommandSpecifier::ABORT_TRANSFER) {
          LOG(ERROR) << "client asked to abort transfer, error code: " << getLast4Byte(request);
          completed = true;
          return;
        }
        if(requestCommandSpec != SdoClientCommandSpecifier::UPLOAD_SEGMENT) {
          LOG(ERROR) << "unexpected client command specifier: " << static_cast<int>(requestCommandSpec);
          abortTransfer(sdoIndex, INVALID_UNKNOWN_COMMAND_SPEC);
          return;
        }
        if(!checkToggleBit(request, sdoIndex, nextToggleBit)) {
          return;
        }

        // Answer request
        CanMsg answer;
        answer.cobId = makeAnsCobId();
        setSdoServerCommandSpecifier(answer, SdoServerCommandSpecifier::UPLOAD_SEGMENT_RESPONSE);
        setSdoSizeBit(answer, true);
        setSdoToggleBit(answer, nextToggleBit);
        nextToggleBit = !nextToggleBit;
        int i;
        for(i = 0;  i < 7 && byteToTransfer < dataSize; i++) {
          answer[1+i] = sdoData[byteToTransfer++];
        }
        setSdoNumberOfBytesOfDataInMsgSdoServer(answer, i, SdoServerCommandSpecifier::UPLOAD_SEGMENT_RESPONSE);
        if(byteToTransfer == dataSize) {
          completed = true;
        }
        setSdoNoMoreSegmentBit(answer, completed);

        card.write(answer);
        if(completed)
          return;
        in();
      }
    });
  }

  void startWrite(CanMsg startMsg) {
    bool expedited = sdoExpeditedTransferIsSet(startMsg);
    SDOIndex sdoIndex = getSdoIndex(startMsg);
    if(!checkODEntryIsWritable(sdoIndex)) {
      return;
    }

    // send expedited write ack
    CanMsg serverAns;
    serverAns.cobId = makeAnsCobId();
    setSdoServerCommandSpecifier(serverAns, SdoServerCommandSpecifier::INITIATE_DOWNLOAD_RESPONSE);
    setSdoIndex(serverAns, sdoIndex);
    card.write(serverAns);

    if(expedited)
      doExpeditedWrite(sdoIndex, startMsg);
    else
      startSegmentedWrite(sdoIndex);
  }

  void doExpeditedWrite(SDOIndex sdoIndex, CanMsg msg) {
    unsigned int dataSize = getSdoNumberOfBytesOfDataInMsg(msg);
    SdoData sdoData = getBytesAsData(msg, 4, 4+dataSize-1);
    updateODEntry(sdoIndex, sdoData);
    completed = true;
  }

  void startSegmentedWrite(SDOIndex sdoIndex) {
    serverCoroutine = std::make_unique<coroutine::push_type>([&, sdoIndex](coroutine::pull_type& in) {
      SdoData sdoData;
      bool nextToggleBit=false;

      while(!completed) {
        CanMsg clientMsg = in.get();
        if(sdoNoMoreSegmentBitIsSet(clientMsg))
          completed = true;
        if(!checkToggleBit(clientMsg, sdoIndex, nextToggleBit)) {
          return;
        }
        const unsigned int sentDataSize = getSdoNumberOfBytesOfDataInMsg(clientMsg);
        SdoData newData = getBytesAsData(clientMsg, 1, sentDataSize);
        sdoData.insert(sdoData.end(), newData.begin(), newData.end());

        // Send segment acknowledge.
        CanMsg serverAns;
        serverAns.cobId = makeAnsCobId();
        setSdoServerCommandSpecifier(serverAns, SdoServerCommandSpecifier::DOWNLOAD_SEGMENT_RESPONSE);
        setSdoToggleBit(serverAns, nextToggleBit);
        card.write(serverAns);

        nextToggleBit = !nextToggleBit;
        if(!completed)
          in();
      }

      // end
      updateODEntry(sdoIndex, sdoData);
    });
  }

  COBId makeAnsCobId() {
    return COBId(nodeId, COBTypeUniqueCode::SDO_TRANSMIT_UNIQUE_CODE);
  }

  bool checkODEntryIsWritable(SDOIndex sdoIndex) {
    if(!objDict.contains(sdoIndex)) {
      LOG(WARNING) << "Received write request for sdo index " << sdoIndex << " but there is no such entry ";
      abortTransfer(sdoIndex, SdoErrorCode::OBJECT_NOT_IN_DICT);
      return false;
    } else if(objDict.getAccess(sdoIndex) == EntryAccess::READONLY) {
      LOG(WARNING) << "Received write request for sdo index " << sdoIndex << " but it is not writable";
      abortTransfer(sdoIndex, SdoErrorCode::WRITE_READONLY);
      return false;
    }
    return true;
  }

  void updateODEntry(SDOIndex sdoIndex, SdoData sdoData) {
    ODEntryValue odEntryValue = objDict.read(sdoIndex);
    SdoEntryWriteVisitor writeVisitor(sdoData);
    // Use the previous value' type in order to update.
    odEntryValue = odEntryValue.apply_visitor(writeVisitor);
    objDict.write(sdoIndex, odEntryValue);
  }

  bool checkToggleBit(CanMsg msg, SDOIndex sdoIndex, bool expected) {
    if(sdoToggleBitIsSet(msg) != expected) {
      LOG(ERROR) << "unexpected toggle bit";
      abortTransfer(sdoIndex, TOGGLE_BIT_NOT_ALTERNATED);
      completed = true;
      return false;
    }
    return true;
  }

  void abortTransfer(SDOIndex sdoIndex, SdoErrorCode sdoErrorCode) {
    CanMsg answer;
    setSdoClientCommandSpecifier(answer, SdoClientCommandSpecifier::ABORT_TRANSFER);
    setSdoIndex(answer, sdoIndex);
    setLast4Byte(answer, sdoErrorCode);
    LOG(WARNING) << "sending abort sdo transfer message with error code " << sdoErrorCode;
    card.write(answer);
    completed = true;
  }

private:
  NodeId nodeId;
  C& card;
  ObjectDictionary& objDict;
  std::unique_ptr<coroutine::push_type> serverCoroutine;
  bool completed;

};

}

#endif // OCO_SDO_SERVER_REQUEST_HPP


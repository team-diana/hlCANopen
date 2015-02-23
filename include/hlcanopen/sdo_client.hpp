// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_HPP
#define OCO_SDO_CLIENT_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/sdo_error.hpp"
#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/can_msg_utils.hpp"
#include "hlcanopen/utils.hpp"

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include "logging/easylogging++.h"

namespace hlcanopen {


  enum class TransStatus {
    NO_TRANS,
    CONT,
    END_OK,
    END_ERR
  };

  enum {
    SDO_TRANSMIT_COB_ID = 0b1011,
    SDO_RECEIVE_COB_ID = 0b1100
  };

  class SdoTransResponse {
    TransStatus status;
  };

  template<class C> class SdoClient {
    typedef boost::coroutines::asymmetric_coroutine<CanMsg> coroutine;

    public:
      SdoClient(NodeId nodeId, C& card) :
      nodeId(nodeId),
      card(card),
      sdoCoroutine(nullptr),
      currentTransStatus(TransStatus::NO_TRANS)
      {

      }

      void readFromNode(SDOIndex sdoIndex) {
        CanMsg readReq;
        readReq.cobId = makeReqCobId();
        setSdoClientCommandSpecifier(readReq, SdoClientCommandSpecifier::INITIATE_UPLOAD);
        setSdoIndex(readReq, sdoIndex);
        card.write(readReq);

        startReadFromNodeCoroutine(sdoIndex);
      }

      void writeToNode(SDOIndex sdoIndex, const SdoData& data) {
        if(data.size() <= 4)
          writeToNodeExpedited(sdoIndex, data);
        else
          writeToNodeSegmented(sdoIndex, data);
      }

      void newMsg(CanMsg msg) {
        BOOST_ASSERT_MSG(sdoCoroutine != nullptr, "coroutine was not started");
        (*sdoCoroutine)(msg);
      }


      TransStatus getTransStatus() {
        return currentTransStatus;
      }

      SdoError getSdoError() {
        return currentSdoError;
      }

      std::vector<unsigned char> getResponseData() {
        return receivedData;
      }

      void cleanForNextRequest() {
        delete sdoCoroutine.release();
        receivedData.clear();
        currentTransStatus = TransStatus::NO_TRANS;
        currentSdoError = SdoError();
      }

      bool transmissionIsEnded() {
        return currentTransStatus == TransStatus::END_OK ||
               currentTransStatus == TransStatus::END_ERR;
      }

  private:
    void startReadFromNodeCoroutine(SDOIndex sdoIndex) {
      currentTransStatus = TransStatus::CONT;
      sdoCoroutine = std::make_unique<coroutine::push_type> (
          [&, sdoIndex](coroutine::pull_type& in){
            CanMsg startServerAns = in.get();
            SdoData readData;
            if(sdoExpeditedTransferIsSet(startServerAns)) {
              unsigned int numberOfBytesSent = getSdoNumberOfBytesOfDataInMsg(startServerAns);
              readData = getBytesAsData(startServerAns, 4, 4+numberOfBytesSent-1);
            } else {
              bool nextToggleBit = false;
              while(true) {
                CanMsg askNextSegmentMsg;
                askNextSegmentMsg.cobId = makeReqCobId();
                setSdoClientCommandSpecifier(askNextSegmentMsg, SdoClientCommandSpecifier::UPLOAD_SEGMENT);
                setSdoIndex(askNextSegmentMsg, sdoIndex);
                setSdoToggleBit(askNextSegmentMsg, nextToggleBit);
                card.write(askNextSegmentMsg);

                in();
                CanMsg serverAns = in.get();
                const unsigned int numberOfBytesSent =
                  getSdoNumberOfBytesOfDataInMsg(serverAns);
                SdoData newData = getBytesAsData(serverAns, 1, numberOfBytesSent);
                readData.insert(readData.end(), newData.begin(), newData.end());
                nextToggleBit = !nextToggleBit;

                if(sdoNoMoreSegmentBitIsSet(serverAns)) {
                  break;
                }
              }
            }
            currentTransStatus = TransStatus::END_OK;
            receivedData = std::move(readData);
            return;
          }
      );
    }

    void writeToNodeExpedited(const SDOIndex sdoIndex, const SdoData& data) {
      CanMsg clientReq;
      clientReq.cobId = makeReqCobId();
      setSdoClientCommandSpecifier(clientReq, SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
      setSdoExpeditedTransfer(clientReq, true);
      setSdoIndex(clientReq, sdoIndex);
      setSdoNumberOfBytesOfDataInMsgSdoClient(clientReq, data.size(),
                                              SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
      for(unsigned int i = 0; i < data.size(); i++) clientReq[4+i] = data[i];

      card.write(clientReq);
      startWriteToNodeExpeditedCoroutine(sdoIndex);
    }

    void startWriteToNodeExpeditedCoroutine(const SDOIndex ) {
      currentTransStatus = TransStatus::CONT;
      sdoCoroutine = std::make_unique<coroutine::push_type> (
          [&](coroutine::pull_type& in){
//             CanMsg ServerAns = in.get();
            in.get();

            // TODO: add check for confirm vs abort
            currentTransStatus = TransStatus::END_OK;
          }
      );
    }

    void writeToNodeSegmented(const SDOIndex sdoIndex, const SdoData& data) {
      CanMsg clientStartReq;
      clientStartReq.cobId = makeReqCobId();
      setSdoClientCommandSpecifier(clientStartReq, SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
      setSdoExpeditedTransfer(clientStartReq, false);
      setSdoIndex(clientStartReq, sdoIndex);
      setSdoSizeBit(clientStartReq, true);
      setLast4Byte(clientStartReq, data.size());
      card.write(clientStartReq);

      startWriteToNodeSegmentedCoroutine(sdoIndex, data);
    }

    void startWriteToNodeSegmentedCoroutine(const SDOIndex sdoIndex, const SdoData& data) {
      currentTransStatus = TransStatus::CONT;
      sdoCoroutine = std::make_unique<coroutine::push_type> (
          [&, sdoIndex, data](coroutine::pull_type& in){
            bool completed = false;
            bool nextToggleBit = false;
            unsigned int currentByteToSend = 0;
            while(true) {
              CanMsg serverAns = in.get();
              // TODO: add check for abort
              SdoServerCommandSpecifier serverAnsComSpec = getSdoServerCommandSpecifier(serverAns);
              if(serverAnsComSpec == SdoServerCommandSpecifier::ABORT_TRANSFER) {
                LOG(ERROR) << "received sdo transfer abort";
                currentTransStatus = TransStatus::END_ERR;
                return;
              }

              if(!completed) {
                CanMsg clientSegment;
                clientSegment.cobId = makeReqCobId();
                setSdoClientCommandSpecifier(clientSegment, SdoClientCommandSpecifier::DOWNLOAD_SEGMENT);
                setSdoToggleBit(clientSegment, nextToggleBit);
                int i = 0;
                for(i = 0; i < 7 && currentByteToSend < data.size(); i++) {
                  clientSegment[1+i] = data[currentByteToSend++];
                }
                setSdoNumberOfBytesOfDataInMsgSdoClient(clientSegment, i, SdoClientCommandSpecifier::DOWNLOAD_SEGMENT);
                if(currentByteToSend == data.size()) {
                  completed = true;
                }
                setSdoNoMoreSegmentBit(clientSegment, completed);
                card.write(clientSegment);
                in();
              } else {
                currentTransStatus = TransStatus::END_OK;
                break;
              }
            }
          }
      );
    }

    bool isAbortMsg(CanMsg msg) {
      if(msg[0] == 0x80) {
        // error code in byte [4-7]
        currentSdoError = SdoError(static_cast<SdoErrorCode>(msg.data & (uint32_t(-1))));
        currentTransStatus = TransStatus::END_ERR;
        return true;
      } else return false;
    }

  COBId makeReqCobId() {
    return COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
  }

  private:
    NodeId nodeId;
    C& card;
    std::unique_ptr<coroutine::push_type> sdoCoroutine;
    SdoData receivedData;
    TransStatus currentTransStatus;
    SdoError currentSdoError;

  };

}

#endif // OCO_SDO_CLIENT_HPP


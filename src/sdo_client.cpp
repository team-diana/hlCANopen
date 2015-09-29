// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/sdo_client.hpp"

namespace hlcanopen {

    SdoClient::SdoClient(NodeId nodeId, CanCard& card) :
        nodeId(nodeId),
        card(card),
        sdoCoroutine(nullptr),
        currentTransStatus(TransStatus::NO_TRANS)
    {

    }

    void SdoClient::readFromNode(SDOIndex sdoIndex) {
        CLOG(INFO, "sdo") << "start read from node: "  << sdoIndex;
        CanMsg readReq;
        readReq.cobId = makeReqCobId();
        setSdoClientCommandSpecifier(readReq, SdoClientCommandSpecifier::INITIATE_UPLOAD);
        setSdoIndex(readReq, sdoIndex);
        card.write(readReq);

        startReadFromNodeCoroutine(sdoIndex);
    }

    /* TODO: check SDO index byte order (inverted) */
    void SdoClient::writeToNode(SDOIndex sdoIndex, const SdoData& data) {
        CLOG(INFO, "sdo") << "start write to node: "  << sdoIndex << " data length: " << data.size();
        if(data.size() <= 4)
            writeToNodeExpedited(sdoIndex, data);
        else
            writeToNodeSegmented(sdoIndex, data);
    }

    void SdoClient::newMsg(CanMsg msg) {
//         BOOST_ASSERT_MSG(sdoCoroutine != nullptr, "coroutine was not started");
        if(sdoCoroutine == nullptr) {
            CLOG(ERROR, "sdo") << "coroutine was not started";
        } else {
            (*sdoCoroutine)(msg);
        }
    }

    TransStatus SdoClient::getTransStatus() {
        return currentTransStatus;
    }

    SdoError SdoClient::getSdoError() {
        return currentSdoError;
    }

    std::vector<unsigned char> SdoClient::getResponseData() {
        return receivedData;
    }

    void SdoClient::cleanForNextRequest() {
        delete sdoCoroutine.release();
        receivedData.clear();
        currentTransStatus = TransStatus::NO_TRANS;
        currentSdoError = SdoError();
    }

    bool SdoClient::transmissionIsEnded() {
        return currentTransStatus == TransStatus::END_OK ||
               currentTransStatus == TransStatus::END_ERR ||
               currentTransStatus == TransStatus::END_TIMEOUT;
    }

    void SdoClient::startReadFromNodeCoroutine(SDOIndex sdoIndex) {
        currentTransStatus = TransStatus::CONT;
        sdoCoroutine = std::make_unique<coroutine::push_type> (
        [&, sdoIndex](coroutine::pull_type& in) {
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

            CLOG(INFO, "sdo") << "read from " << sdoIndex << " completed";
            currentTransStatus = TransStatus::END_OK;
            receivedData = std::move(readData);
            return;
        }
                       );
    }

    void SdoClient::writeToNodeExpedited(const SDOIndex sdoIndex, const SdoData& data) {
        BOOST_ASSERT_MSG(data.size() <= 4, "data lenght cannot be > 4 in expedited write");
        CanMsg clientReq;
        clientReq.cobId = makeReqCobId();
        setSdoClientCommandSpecifier(clientReq, SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
        setSdoExpeditedTransfer(clientReq, true);
        setSdoIndex(clientReq, sdoIndex);
        setSdoNumberOfBytesOfDataInMsgSdoClient(clientReq, data.size(),
                                                SdoClientCommandSpecifier::INITIATE_DOWNLOAD);
        setSdoSizeBit(clientReq, true);
        for(unsigned int i = 0; i < data.size(); i++) clientReq[4+i] = data[i];

        card.write(clientReq);
        startWriteToNodeExpeditedCoroutine(sdoIndex);
    }

    void SdoClient::startWriteToNodeExpeditedCoroutine(const SDOIndex sdoIndex) {
        currentTransStatus = TransStatus::CONT;
        sdoCoroutine = std::make_unique<coroutine::push_type> (
        [&](coroutine::pull_type& in) {
//             CanMsg ServerAns = in.get();
            in.get();

            // TODO: add check for confirm vs abort
            CLOG(INFO, "sdo") << "expedited write to " << sdoIndex << " completed";
            currentTransStatus = TransStatus::END_OK;
        }
                       );
    }

    void SdoClient::writeToNodeSegmented(const SDOIndex sdoIndex, const SdoData& data) {
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

    void SdoClient::startWriteToNodeSegmentedCoroutine(const SDOIndex sdoIndex, const SdoData& data) {
        currentTransStatus = TransStatus::CONT;
        sdoCoroutine = std::make_unique<coroutine::push_type> (
        [&, sdoIndex, data](coroutine::pull_type& in) {
            bool completed = false;
            bool nextToggleBit = false;
            unsigned int currentByteToSend = 0;
            while(true) {
                CanMsg serverAns = in.get();
                // TODO: add check for abort
                SdoServerCommandSpecifier serverAnsComSpec = getSdoServerCommandSpecifier(serverAns);
                if(serverAnsComSpec == SdoServerCommandSpecifier::ABORT_TRANSFER) {
                    CLOG(ERROR, "sdo") << "received sdo transer abort during write ";
                    currentTransStatus = TransStatus::END_ERR;
                    return;
                }

                if(!completed) {
                    CanMsg clientSegment;
                    clientSegment.cobId = makeReqCobId();
                    setSdoClientCommandSpecifier(clientSegment, SdoClientCommandSpecifier::DOWNLOAD_SEGMENT);
                    setSdoToggleBit(clientSegment, nextToggleBit);
                    nextToggleBit = !nextToggleBit;
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
                    CLOG(INFO, "sdo") << "write to " << sdoIndex << " completed";
                    currentTransStatus = TransStatus::END_OK;
                    break;
                }
            }
        }
                       );
    }

    bool SdoClient::isAbortMsg(CanMsg msg) {
        if(msg[0] == 0x80) {
            // error code in byte [4-7]
            currentSdoError = SdoError(static_cast<SdoErrorCode>(msg.data & (uint32_t(-1))));
            currentTransStatus = TransStatus::END_ERR;
            return true;
        } else return false;
    }

    COBId SdoClient::makeReqCobId() {
        return COBId(nodeId, COBTypeUniqueCode::SDO_RECEIVE_UNIQUE_CODE);
    }

}


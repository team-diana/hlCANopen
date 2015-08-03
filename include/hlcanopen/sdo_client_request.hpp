// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_CLIENT_REQUEST_HPP
#define OCO_SDO_CLIENT_REQUEST_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/utils.hpp"
#include "hlcanopen/sdo_error.hpp"

namespace hlcanopen {

// typedef boost::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, std::string> MsgContent;

// class SdoClientRequest : protected boost::static_visitor<void>

class SdoClientReadRequest;
class SdoClientWriteRequest;

class SdoRequestVisitor {
public:
    virtual ~SdoRequestVisitor() {}
    virtual void visitSdoClientRequestStart(const SdoClientReadRequest& request) = 0;
    virtual void visitSdoClientRequestStart(const SdoClientWriteRequest& request) = 0;
    virtual void visitSdoClientRequestEnd(SdoClientReadRequest& request) = 0;
    virtual void visitSdoClientRequestEnd(SdoClientWriteRequest& request) = 0;
    virtual void visitSdoClientRequestTimeout(SdoClientReadRequest& request) = 0;
    virtual void visitSdoClientRequestTimeout(SdoClientWriteRequest& request) = 0;
};

class SdoClientRequest {
public:
    SdoClientRequest(SDOIndex sdoIndex, long timeout) :
        sdoIndex(sdoIndex),
        timestamp(getTimestamp() + timeout) {}

    virtual ~SdoClientRequest() {}

    virtual void visitStart(SdoRequestVisitor& visitor) = 0;
    virtual void visitEnd(SdoRequestVisitor& visitor) = 0;
    virtual void visitTimeout(SdoRequestVisitor& visitor) = 0;

    SDOIndex sdoIndex;

    long timestamp;

private:
    long getTimestamp() {
        auto ts = std::chrono::duration_cast< std::chrono::milliseconds >(
                      std::chrono::system_clock::now().time_since_epoch())
                  .count();
        return ts;
    }
};

class SdoClientReadRequest : public SdoClientRequest {
public:
    // The value of typeToken is not actually used, only its type it's needed.
    SdoClientReadRequest(SDOIndex sdoIndex, long timeout) :
        SdoClientRequest(sdoIndex, timeout) {}
    virtual ~SdoClientReadRequest() {}

    virtual void completeRequest(const SdoData& sdoData) = 0;
    virtual void completeRequestWithFail(const SdoError& error) = 0;
    virtual void completeRequestWithTimeout() = 0;

    virtual void visitStart(SdoRequestVisitor& visitor) override {
        visitor.visitSdoClientRequestStart(*this);
    }
    virtual void visitEnd(SdoRequestVisitor& visitor) override {
        visitor.visitSdoClientRequestEnd(*this);
    }
    virtual void visitTimeout(SdoRequestVisitor& visitor) override {
        visitor.visitSdoClientRequestTimeout(*this);
    }

};

class SdoClientWriteRequest : public SdoClientRequest {
public:
    // The value of typeToken is not actually used, only its type it's needed.
    SdoClientWriteRequest(SDOIndex sdoIndex, SdoData sdoData, long timeout) :
        SdoClientRequest(sdoIndex, timeout),
        sdoData(sdoData) {}
    virtual ~SdoClientWriteRequest() {};

    virtual void completeRequest() = 0;
    virtual void completeRequestWithFail(const SdoError& error) = 0;
    virtual void completeRequestWithTimeout() = 0;

    virtual void visitStart(SdoRequestVisitor& visitor) override {
        visitor.visitSdoClientRequestStart(*this);
    }
    virtual void visitEnd(SdoRequestVisitor& visitor) override {
        visitor.visitSdoClientRequestEnd(*this);
    }
    virtual void visitTimeout(SdoRequestVisitor& visitor) override {
        visitor.visitSdoClientRequestTimeout(*this);
    }

    SdoData sdoData;
};


}

#endif // OCO_SDO_CLIENT_REQUEST_HPP


// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/sdo_data_converter.hpp"

namespace hlcanopen {

template<> int32_t convertSdoData<int32_t> (const SdoData& d) {
    int32_t value = ((uint32_t)d[0])        |
                    ((uint32_t)d[1] << 8)   |
                    ((uint32_t)d[2] << 16)  |
                    ((uint32_t)d[3] << 24);
    return value;
}

template<> uint32_t convertSdoData<uint32_t> (const SdoData& d) {
    uint32_t value = ((uint32_t)d[0])        |
                     ((uint32_t)d[1] << 8)   |
                     ((uint32_t)d[2] << 16)  |
                     ((uint32_t)d[3] << 24);
    return value;
}

template<> uint8_t convertSdoData<uint8_t> (const SdoData& d) {
    return convertSdoData< uint32_t >(d);
}

template<> std::string convertSdoData<std::string> (const SdoData& d) {
    return std::string(d.begin(), d.end());
}

template<> SdoData convertValue(const uint32_t& v) {
    SdoData data;
    data.resize(4);
    data[0] = (v & 0x000000ff);
    data[1] = (v & 0x0000ff00) >> 8;
    data[2] = (v & 0x00ff0000) >> 16;
    data[3] = (v & 0xff000000) >> 24;
    return data;
}

template<> SdoData convertValue(const uint16_t& v) {
    return convertValue<uint32_t>(v);
}

template<> SdoData convertValue(const uint8_t& v) {
    return convertValue<uint32_t>(v);
}

template<> SdoData convertValue(const int32_t& v) {
    SdoData data;
    // TODO: check if same effect of uint32_t and use that code
    data.resize(4);
    data[0] = (v & 0x000000ff);
    data[1] = (v & 0x0000ff00) >> 8;
    data[2] = (v & 0x00ff0000) >> 16;
    data[3] = (v & 0xff000000) >> 24;
    return data;
}

template<> SdoData convertValue(const std::string& v) {
    SdoData data;
    data.resize(v.size());
    std::copy(v.begin(), v.end(), data.begin());
    return data;
}

}

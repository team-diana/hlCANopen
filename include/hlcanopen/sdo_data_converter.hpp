// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_DATA_CONVERTER_HPP
#define OCO_SDO_DATA_CONVERTER_HPP

#include "hlcanopen/types.hpp"
#include "hlcanopen/utils.hpp"

#include <boost/core/ignore_unused.hpp>

#include <cstdint>
#include <algorithm>

namespace hlcanopen {

//     template<typename T, typename D> T convert_data(D&& data);
template<typename T> T convertSdoData(const SdoData& d) {
    boost::ignore_unused(d);
    NOT_IMPLEMENTED_YET;
    // TODO: how to disable this at compile-time?
//       static_assert(false, "unable to convert to this type.");
    T r;
    return r;
}

template<> int32_t convertSdoData<int32_t> (const SdoData& d);

template<> uint32_t convertSdoData<uint32_t> (const SdoData& d);

template<> uint8_t convertSdoData<uint8_t> (const SdoData& d);

template<> std::string convertSdoData<std::string> (const SdoData& d);

template<typename T> SdoData convertValue(const T& v) {
    IGNORE(v)
    NOT_IMPLEMENTED_YET
    return T();
}

template<> SdoData convertValue(const uint32_t& v);

template<> SdoData convertValue(const uint16_t& v);

template<> SdoData convertValue(const uint8_t& v);

template<> SdoData convertValue(const int32_t& v);

template<> SdoData convertValue(const std::string& v);

}

#endif // OCO_SDO_DATA_CONVERTER_HPP


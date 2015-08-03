// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_SDO_ENTRY_CONVERTER_VISITOR_HPP
#define OCO_SDO_ENTRY_CONVERTER_VISITOR_HPP

#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/types.hpp"

#include <boost/variant/static_visitor.hpp>

namespace hlcanopen {

class SdoEntryConverterVisitor : public boost::static_visitor<SdoData>
{
public:
    SdoData operator()(int32_t i) const;
    SdoData operator()(uint32_t i) const;
    SdoData operator()(const std::string & str) const;
};

class SdoEntryWriteVisitor : public boost::static_visitor<ODEntryValue> {
public:
    SdoEntryWriteVisitor(SdoData data);

    ODEntryValue operator()(int32_t i);
    ODEntryValue operator()(uint32_t i);
    ODEntryValue operator()(const std::string& str);

private:
    void checkAndFixSize(SdoData& d, unsigned int nBytes);

private:
    SdoData data;
};

}

#endif // OCO_SDO_ENTRY_CONVERTER_VISITOR_HPP


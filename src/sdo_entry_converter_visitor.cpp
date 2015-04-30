// Copyright (C) 2015 team-diana MIT license

#include "hlcanopen/sdo_data_converter.hpp"
#include "hlcanopen/sdo_entry_converter_visitor.hpp"
#include "hlcanopen/object_dictionary.hpp"
#include "hlcanopen/types.hpp"

#include <boost/variant/static_visitor.hpp>

#include "hlcanopen/logging/easylogging++.h"

namespace hlcanopen {

SdoData SdoEntryConverterVisitor::operator()(uint32_t i) const
{
  return convertValue(i);
}

SdoData SdoEntryConverterVisitor::operator()(int32_t i) const
{
  return convertValue(i);
}

SdoData SdoEntryConverterVisitor::operator()(const std::string& str) const
{
  return convertValue(str);
}

SdoEntryWriteVisitor::SdoEntryWriteVisitor(SdoData data) :
data(data) {}

ODEntryValue SdoEntryWriteVisitor::operator()(uint32_t)
{
  checkAndFixSize(data, 4);
  return convertSdoData<uint32_t>(data);
}

ODEntryValue SdoEntryWriteVisitor::operator()(int32_t)
{
  checkAndFixSize(data, 4);
  return convertSdoData<int32_t>(data);
}

ODEntryValue SdoEntryWriteVisitor::operator()(const std::string&)
{
  return convertSdoData<std::string>(data);
}

void SdoEntryWriteVisitor::checkAndFixSize(SdoData& d, unsigned int nBytes)
{
  if(d.size() != nBytes) {
    LOG(WARNING) << "resizing sdo data from " << static_cast<unsigned int>(d.size()) << " to " << nBytes;
    d.resize(nBytes);
  }
}

}


// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_OBJECT_DICTIONARY_HPP
#define OCO_OBJECT_DICTIONARY_HPP

#include "types.hpp"

#include "boost/blank.hpp"
#include "boost/variant.hpp"

#include <map>
#include <string>

namespace hlcanopen {

  typedef boost::variant<uint32_t, int32_t, std::string> ODEntryValue;
  enum class EntryAccess : unsigned char {
    READONLY,
    WRITEONLY,
    READWRITE
  };
  struct ODEntry {
    EntryAccess access;
    ODEntryValue value;
  };

  class ObjectDictionary {
  public:
    void write(SDOIndex index, ODEntryValue value) {
      ODEntry entry = map[index];
      entry.value = value;
      map[index] = entry;
    }

    bool writeCheckAccess(SDOIndex index, ODEntryValue value) {
      ODEntry entry = map[index];
      if(entry.access == EntryAccess::READONLY) {
        return false;
      } else entry.value = value;
    }

    ODEntryValue read(SDOIndex index) {
      return map[index].value;
    }

    bool contains(SDOIndex index) {
      return map.find(index) != map.end();
    }

    EntryAccess getAccess(SDOIndex index) {
      return map[index].access;
    }

    void setAccess(SDOIndex index, EntryAccess access) {
      ODEntry entry = map[index];
      entry.access = access;
      map[index] = entry;
    }

  private:
    std::map<SDOIndex, ODEntry, SDOIndexCompare> map;
  };

}

#endif // OCO_OBJECT_DICTIONARY_HPP


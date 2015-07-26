// Copyright (C) 2015 team-diana MIT license

#ifndef OCO_UTILS_HPP
#define OCO_UTILS_HPP

#include "boost/assert.hpp"

#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <chrono>
#include <iomanip>

#define NOT_IMPLEMENTED_YET assert(false&&("NOT IMPLEMENTED YET"));

#define ASSERT_MSG_COUT(cond, msg_exp) do { \
  std::ostringstream str; \
  str << msg_exp; \
  if(cond) { \
    std::cout << "ASSERT ERROR: " << str.str() << std::endl; \
    assert(false); \
  } \
} while (0)

template <typename T> void IGNORE(const T& t) {&t;}

namespace hlcanopen {

  // Ignore variable in order to remove warnings.
//   void ignore(...) {
//   }


  template<typename K, typename V> typename std::map<K, V>::iterator
  get_or_create(std::map<K, V>& map, const K& key, std::function<V()> creator) {
    auto it = map.find(key);
    if(it != map.end()) {
      return it;
    } else {
      map.insert(std::make_pair(key, creator()));
      return map.find(key);
    }
  }

  template<typename K, class V> typename std::map<K, V>::iterator
  get_or_create(std::map<K, V>& map, const K& key) {
    std::function<V()> f = []{return V();};
    return get_or_create(map, key, f);
  }

  static std::ostream& hexUppercase(std::ostream& out) {
    return out << std::hex << std::setfill('0')
        << std::uppercase
        << std::setw(2);
  }

#if 0
  long getTimestamp() {
    auto ts = std::chrono::duration_cast< std::chrono::milliseconds >(
		  std::chrono::system_clock::now().time_since_epoch())
		  .count();
    return ts;
  }
#endif
}


#endif // OCO_UTILS_HPP


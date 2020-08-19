#pragma once

#include <limits>
#include <stddef.h>
#include <string>
#include <tuple>
#include <unordered_map>

#include "../../third-party/Empirical/source/tools/string_utils.h"

#include "OccupancyCap.hpp"

namespace uit {

class OccupancyCaps {

  std::unordered_map<std::string, OccupancyCap> lookup;
  mutable std::mutex mutex;

public:

  OccupancyCaps() = default;

  OccupancyCaps(const OccupancyCaps&) { ; }

  OccupancyCaps & operator=(const OccupancyCaps&) { return *this; }

  OccupancyCap & Get(
    const std::string & name,
    const size_t maximum_occupancy=std::numeric_limits<size_t>::max()
  ) {

    const std::lock_guard guard{mutex};

    if (!lookup.count(name)) {
      auto res = lookup.emplace(
        std::piecewise_construct,
        std::tuple{name},
        std::tuple{maximum_occupancy}
      );
      return res.first->second;
    } else {
      auto & res = lookup.at(name);
      emp_assert(
        res.GetMaximumOccupancy() == maximum_occupancy,
        [](){ error_message_mutex.lock(); return "locked"; }(),
        maximum_occupancy,
        res.GetMaximumOccupancy()
      );
      return res;
    }

  }

};

} // namespace uit
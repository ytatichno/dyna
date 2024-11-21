#ifndef _REGION_ACTUAL_MAP_HPP_
#define _REGION_ACTUAL_MAP_HPP_

#include <unordered_map>
#include "ContextStringParser.h"

namespace dyna {


// hide under conditional compilation
enum class ActualStatus : uint8_t {
  INACTUAL,
  ACTUAL_HOST,
  ACTUAL_REGION,
  ACTUAL_BOTH
};

struct ActualInfo {
  ActualStatus status;
  BasicString* contextString;
};

/// TODO hide it behind conditional compilation
typedef std::unordered_map<long, ActualInfo> RegionActualMap;

}

#endif
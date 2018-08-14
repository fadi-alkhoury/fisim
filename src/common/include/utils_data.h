#ifndef FISIM_COMMON_INCLUDE_COMMON_UTILS_DATA_H_
#define FISIM_COMMON_INCLUDE_COMMON_UTILS_DATA_H_

#include "types.h"
#include <vector>

namespace fisim {
namespace utils {

class GetCalMonth final {
  public:
    GetCalMonth() = default;
    GetCalMonth(CalendarMonth calMonthInit) : _monthInit{calMonthInit} {}
    CalendarMonth operator()(tUint nMonthNow) { return incrementMonth(_monthInit, nMonthNow); }

  private:
    CalendarMonth _monthInit;
};

template <typename T> std::vector<T> makeReservedVector(tSize capacity) {
    std::vector<T> newVector;
    newVector.reserve(capacity);

    return newVector;
}

} // namespace utils
} // namespace fisim
#endif /* FISIM_COMMON_INCLUDE_COMMON_UTILS_DATA_H_ */

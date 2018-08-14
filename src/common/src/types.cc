#include "types.h"

#include <cassert>
#include <stdexcept>

namespace fisim {

CalendarMonth incrementMonth(CalendarMonth month, tInt n) {
    assert(month != CalendarMonth::none);

    if (n < 0) {
        return static_cast<CalendarMonth>((static_cast<tInt>(month) - 1 + (12 + n % 12)) % 12 + 1);
    }

    return static_cast<CalendarMonth>((static_cast<tInt>(month) + n - 1) % 12 + 1);
}

tUint calMonthsDiff(CalendarMonth monthStart, CalendarMonth monthEnd) {
    if (monthStart == CalendarMonth::none || monthEnd == CalendarMonth::none) {
        throw std::domain_error("Invalid months.");
    }

    const tInt monthEndInteger = static_cast<tInt>(monthEnd);
    const tInt monthStartInteger = static_cast<tInt>(monthStart);
    tInt diff = monthEndInteger - monthStartInteger;

    if (monthEndInteger < monthStartInteger) {
        diff += 12;
    }

    return static_cast<tUint>(diff);
}

} // namespace fisim

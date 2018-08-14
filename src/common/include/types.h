#ifndef FISIM_TYPES_H_
#define FISIM_TYPES_H_

#include <limits>

namespace fisim {

using tUint = unsigned;
using tSize = std::size_t;
using tInt = int;
using tLong = long;
using tFloat = float;
using tAmount = double; ///< to use for money amounts

namespace constants {

template <typename T> constexpr T NaN = std::numeric_limits<T>::quiet_NaN();
template <typename T> constexpr T limitMax = std::numeric_limits<T>::max();

constexpr tFloat tolerance = 0.001f;

} // namespace constants

///
/// ID to identify cashflows and loans and their associations
///
struct Id {
    tInt idCashflow = 0; ///< non-zero indicates a cashflow, or that a loan is associated with a cashflow
    tInt idLoan = 0;     ///< non-zero indicates a loan
    tInt idTax = 0;      ///< identifies the associated tax account. Zero indicates no tax. A tax return transac is characterized by a non zero idTax and all other IDs zero

    bool isLoan() const { return idLoan != 0 && idCashflow == 0; }
    bool isTaxDeductibleInterest() const { return idTax != 0 && idLoan != 0; }
};

enum class CalendarMonth : tUint { jan = 1u, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec, none };

///
/// Increment or decrement a calendar month
///
CalendarMonth incrementMonth(CalendarMonth month, tInt n);

tUint calMonthsDiff(CalendarMonth monthStart, CalendarMonth monthEnd);

///
/// A class intended to be used within containers instead of the bool template specialization
///
class Bool {
  public:
    Bool(bool value = false) : _value(value) {}

    operator bool() const { return _value; }

    bool *operator&() { return &_value; }
    bool const *operator&() const { return &_value; }

  private:
    bool _value;
};

} // namespace fisim

#endif /* FISIM_TYPES_H_ */

#ifndef FISIM_CXX_LOAN_CXX_LOANHOLIDAY_H_
#define FISIM_CXX_LOAN_CXX_LOANHOLIDAY_H_

#include "loan.h"
#include <memory>

namespace fisim {
namespace loan {

class LoanHoliday final {
  public:
    struct Params {
        tUint nMonthsPerYear; ///< The holiday can have either a maximum amount per year or for the whole term
        tUint nMonthsYearRemaining;
        tUint nMonthsTermRemaining; ///< should be initialized using nMonthsMaxTerm
        tUint nMonthsStartOffset;   // offset needed so the reset months can be found
        bool isAllowConsecutive;    ///< If false, consecutive months of holiday are not allowed
        bool isSequenceStarted;
        bool isExtendTerm;      ///< If true, the loan's term is extended
        tUint* pNumMonthsToEnd; ///< pointer to the loan span so it can be extended
        Loan* pLoan;
    };

    LoanHoliday(std::unique_ptr<Params> pParams);

    void apply();

    bool isHolidayPossible() const;

    bool isAnnualReset(tUint nMonthsDone) const;

    void annualReset();

    void resetSequence();

    void makeCheckPoint();
    void resetToCheckPoint();

  private:
    std::unique_ptr<Params> _pParams;

    tUint _nMonthsYearRemainingCheckPoint = 0u;
    tUint _nMonthsTermRemainingCheckPoint = 0u;
    bool _isSequenceStartedCheckPoint = false;
};

} // namespace loan
} // namespace fisim

#endif /* FISIM_CXX_LOAN_CXX_LOANHOLIDAY_H_ */

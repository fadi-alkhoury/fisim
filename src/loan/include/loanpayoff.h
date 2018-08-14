#ifndef FISIM_LOAN_LOANPAYOFF_H_
#define FISIM_LOAN_LOANPAYOFF_H_

#include "loan.h"
#include "repayment_option.h"
#include <memory>

namespace fisim {
namespace loan {

class LoanPayoff final {
  public:
    struct Params {
        tUint nMonthsPenalty; ///< Should be set to zero if penaltyFactor needs to be used. Means that the penalty is interest payments for a number of months.
        tFloat penaltyFactor; ///< Has no effect if nMonthsPenalty > 0. Means that the penalty is a percent of the remaining principle. If set to 1 (and nMonthsPenalty is 0), then
                              ///< payoff is uninteresting as long as the penalty applies.
        tUint nMonthsThresh;  ///< This threshold is the number of months until penaltyFactor no longer applies (eg. if the penalty applies only if the loan is prepaid within 10
                              ///< years).
        bool isFixedNmonthsPenalty; ///< If true, nMonthsPenalty is fixed up to the number of remaining months in the term. Otherwise the number of penalty months decrements each
                                    ///< month.

        tRepaymentOption repaymentOption; ///< same as in LoanSettings
        tAmount monthlyPayment;           ///< same as in LoanSettings
    };

    explicit LoanPayoff(std::unique_ptr<Params>&& pParams);

    struct PenaltyInfo {
        tAmount amount = 0.0;
        tAmount amountSaved = 0.0;
    };

    /// Finds the payoff penalty and the interest cost saved if using the option
    ///	Notes:
    ///	 - Only the interest cost up to the loan term is considered.
    ///	 - The actual savings can be greater in case the loan is not fully repaid in the end.
    ///	 - For a months-specified penalty, the number of penalty months will not exceed the number of months left in the loan
    ///
    PenaltyInfo payOffPenalty(tAmount amountLoanNow, tUint nMonthsDone, tUint nMonthsLoan, tFloat interestRate) const;

    Loan::LoanStateChange processPayOff(loan::Loan& loan, tAmount penalty) const;

  private:
    std::unique_ptr<Params> _pParams;
};

} // namespace loan
} // namespace fisim

#endif /* FISIM_LOAN_LOANPAYOFF_H_ */

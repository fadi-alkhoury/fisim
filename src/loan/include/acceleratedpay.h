#ifndef FISIM_LOAN_ACCELERATEDPAY_H_
#define FISIM_LOAN_ACCELERATEDPAY_H_

#include "loan.h"
#include <memory>
#include <vector>

namespace fisim {
namespace loan {

class AcceleratedPay final {
  public:
    struct Params {
        tFloat annualAllowanceFactor; ///< Maximum annual amount as a portion (depending on isFactorOfStartingAmount either of initial or remaining amount)
        tAmount thisYearRemaining;    ///< The usable allowance in the first month of simulating the loan (sim month 0 or the start of the loan if it falls after sim start)
        tUint nMonthsStartOffset;
        tAmount minAmount;  ///< The minimum amount per payment
        bool isAnnualReset; ///< If true, the allowance resets after each 12 months of loan
        bool isFactorOfStartingAmount;
        std::vector<Bool> monthsAllowed; ///< The months where the extra payments can be made.
    };

    explicit AcceleratedPay(std::unique_ptr<Params>&& pParams);

    /** Reduces the loan amount and decreases the remaining allowance for the year */
    tAmount processUnschedPayment(loan::Loan& loan, tAmount amount);

    /** Finds the unsched payment amount that is possible now */
    tAmount amountUnschedPossible(tAmount amountAvailable, tUint nMonthsDone);

    /** Checks whether, according to paramsAcc, the allowance can be reset now */
    bool isAnnualReset(tUint nMonthsDone);

    /** Resets the unsched payment annual allowance, assuming a loan year is now complete*/
    void annualReset(tAmount amountNowLoan, tAmount amountInitLoan);

    /** Accessor to help with testing*/
    tAmount thisYearRemaining() const { return _pParams->thisYearRemaining; }

    void makeCheckPoint() { _thisYearRemainingCheckPoint = _pParams->thisYearRemaining; }
    void resetToCheckPoint() { _pParams->thisYearRemaining = _thisYearRemainingCheckPoint; }

  private:
    std::unique_ptr<Params> _pParams;
    tAmount _thisYearRemainingCheckPoint = 0.0;
};

} // namespace loan
} // namespace fisim

#endif /* FISIM_LOAN_ACCELERATEDPAY_H_ */

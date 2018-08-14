#include "acceleratedpay.h"
#include <cassert>
#include <utility>

namespace fisim {
namespace loan {

AcceleratedPay::AcceleratedPay(std::unique_ptr<AcceleratedPay::Params>&& pParams) : _pParams(std::move(pParams)) {}

tAmount AcceleratedPay::amountUnschedPossible(tAmount amountAvailable, tUint nMonthsDone) {
    if (!_pParams->monthsAllowed[nMonthsDone]) {
        return 0.0;
    }

    bool isMinAmountAllowed = (_pParams->minAmount <= _pParams->thisYearRemaining);
    if (!isMinAmountAllowed) {
        return 0;
    }

    bool isMinAmountSatisfied = (_pParams->minAmount <= amountAvailable);
    bool isWithinLimit = (amountAvailable <= _pParams->thisYearRemaining);

    tAmount amountPossible = 0;
    if (isMinAmountSatisfied && isWithinLimit) {
        amountPossible = amountAvailable;
    } else if (isMinAmountSatisfied && !isWithinLimit) {
        amountPossible = _pParams->thisYearRemaining;
    }

    return amountPossible;
}

tAmount AcceleratedPay::processUnschedPayment(loan::Loan& loan, tAmount amount) {
    Loan::LoanStateChange stateChange;
    stateChange.principle = amount;
    loan.processStateChange(stateChange);

    _pParams->thisYearRemaining -= stateChange.principle;
    assert(_pParams->thisYearRemaining >= 0);

    return stateChange.principle;
}

bool AcceleratedPay::isAnnualReset(tUint nMonthsDone) {
    tInt nMonthsEffective = (nMonthsDone + _pParams->nMonthsStartOffset);

    bool isResetMonth = ((nMonthsEffective % 12 == 0) && (nMonthsEffective > 0));
    return (isResetMonth && _pParams->isAnnualReset);
}

void AcceleratedPay::annualReset(tAmount amountNowLoan, tAmount amountInitLoan) {
    if (_pParams->isFactorOfStartingAmount) {
        _pParams->thisYearRemaining = amountInitLoan * _pParams->annualAllowanceFactor;
    } else {
        _pParams->thisYearRemaining = amountNowLoan * _pParams->annualAllowanceFactor;
    }
}

} // namespace loan
} // namespace fisim

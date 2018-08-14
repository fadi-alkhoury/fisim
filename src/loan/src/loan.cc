#include "loan.h"
#include <cassert>

namespace fisim {
namespace loan {

void Loan::processStateChange(LoanStateChange& stateChange) {
    assert(amount > 0);

    interestPaid += stateChange.interest;

    if (stateChange.principle >= amount - constants::tolerance) {
        stateChange.principle = amount;
        amount = 0.0;
    } else {
        amount -= stateChange.principle; // negative amortization possible
    }
}

void Loan::makeCheckPoint() {
    _amountCheckPoint = amount;
    _interestPaidCheckPoint = interestPaid;
}

void Loan::resetToCheckPoint() {
    amount = _amountCheckPoint;
    interestPaid = _interestPaidCheckPoint;
}

} // namespace loan
} // namespace fisim

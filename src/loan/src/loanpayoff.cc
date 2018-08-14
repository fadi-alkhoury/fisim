#include "loanpayoff.h"
#include <cassert>

namespace {
using tRepaymentOption = fisim::loan::tRepaymentOption;
using tAmount = fisim::tAmount;
using tUint = fisim::tUint;
using tFloat = fisim::tFloat;

tAmount totalAmortizedInterest(
    tAmount amountNow,
    tFloat interestRate,
    tRepaymentOption repaymentOption,
    tAmount payment, ///< the monthly payment for tRepaymentOption::monthlyPayment
    tUint nMonthsCalc);
} // namespace

namespace fisim {
namespace loan {

LoanPayoff::LoanPayoff(std::unique_ptr<LoanPayoff::Params>&& pParams) : _pParams(std::move(pParams)) {}

Loan::LoanStateChange LoanPayoff::processPayOff(loan::Loan& loan, tAmount penalty) const {
    Loan::LoanStateChange stateChange;
    stateChange.interest = penalty;
    stateChange.principle = loan.amount;

    loan.processStateChange(stateChange);

    return stateChange;
}

LoanPayoff::PenaltyInfo LoanPayoff::payOffPenalty(tAmount amountLoanNow, tUint nMonthsDone, tUint nMonthsLoan, tFloat interestRate) const {
    PenaltyInfo payoffInfo;
    tUint nMonthsLeft = nMonthsLoan - nMonthsDone;

    if (nMonthsDone < _pParams->nMonthsThresh) {
        if (_pParams->nMonthsPenalty > 0) {
            tUint nMonthsPenaltyEff;
            if (_pParams->isFixedNmonthsPenalty == false) {
                nMonthsPenaltyEff = _pParams->nMonthsPenalty - nMonthsDone;
            } else {
                nMonthsPenaltyEff = std::min(_pParams->nMonthsPenalty, nMonthsLeft);
            }

            payoffInfo.amount = totalAmortizedInterest(amountLoanNow, interestRate, _pParams->repaymentOption, _pParams->monthlyPayment, nMonthsPenaltyEff);
        } else {
            // penalty is a % of amount
            payoffInfo.amount = amountLoanNow * _pParams->penaltyFactor;
        }
    } else {
        // No penalty, just the last month's interest.
        // Note: if nMonthsDone is less than thresh and the last month interest is needed, then nMonthsPenalty should be
        // 1
        payoffInfo.amount = amountLoanNow * interestRate;
    }

    auto costTillEndOfTerm = totalAmortizedInterest(amountLoanNow, interestRate, _pParams->repaymentOption, _pParams->monthlyPayment, nMonthsLeft);
    payoffInfo.amountSaved = costTillEndOfTerm - payoffInfo.amount;
    // gives the savings only up to end of this loan. The actual savings can be greater in case the loan is not fully
    // repaid in the end.

    return payoffInfo;
}

} // namespace loan
} // namespace fisim

namespace {

tAmount totalAmortizedInterest(
    tAmount amountNow,
    tFloat interestRate,
    tRepaymentOption repaymentOption,
    tAmount payment, ///< the monthly payment for tRepaymentOption::monthlyPayment
    tUint nMonthsCalc) {
    tAmount amountPenalty = 0.0;
    switch (repaymentOption) {
        case tRepaymentOption::monthlyPayment: {
            tAmount interest, principle;
            for (auto i = 0u; i < nMonthsCalc; ++i) {
                interest = interestRate * amountNow;
                amountPenalty += interest;

                if (interest > payment) {
                    principle = 0.0;
                } else {
                    principle = payment - interest;
                }

                if (amountNow > principle) {
                    amountNow -= principle;
                } else {
                    break;
                }
            }

            break;
        }
        default: { // interest-only
            amountPenalty = interestRate * amountNow * nMonthsCalc;
        }
    }

    return amountPenalty;
}

} // namespace

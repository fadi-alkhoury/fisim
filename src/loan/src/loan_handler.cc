#include "loan_handler.h"
#include <cassert>
#include <memory>

namespace {
using LoanStateChange = fisim::loan::Loan::LoanStateChange;
using PaymentBreakdown = fisim::loan::PaymentBreakdown;
using tRepaymentOption = fisim::loan::tRepaymentOption;
using Loan = fisim::loan::Loan;
using LoanSettings = fisim::loan::LoanSettings;
using LoanHoliday = fisim::loan::LoanHoliday;
using tAmount = fisim::tAmount;
using tFloat = fisim::tFloat;
using tUint = fisim::tUint;

// keeps the payment within the cap. negative amortization can be needed.
void validatePayment(tAmount monthlyPaymentCap, LoanStateChange& payment);

PaymentBreakdown processMonthNormal(LoanSettings const& settings, tUint nMonthsDone, Loan& loan);

PaymentBreakdown processFirstMonth_imp(LoanSettings const& settings, Loan& loan, tFloat fracInterest, tFloat fracRepayment);

tAmount processRepayment_imp(Loan& loan);

tAmount processHoliday(tFloat interestRate, tAmount monthlyPaymentCap, Loan& loan, LoanHoliday& loanHoliday);

} // namespace

namespace fisim {
namespace loan {

LoanHandlerNor::LoanHandlerNor(std::unique_ptr<ParamsHandler>&& pParamsHandler) : _pParamsHandler{std::move(pParamsHandler)} {}

PaymentBreakdown LoanHandlerNor::processMonth(tUint nMonthsDone) {
    auto& paramsHandler = *_pParamsHandler;

    return processMonthNormal(paramsHandler.settings, nMonthsDone, paramsHandler.loan);
}

tAmount LoanHandlerNor::processRepayment() {
    return processRepayment_imp(_pParamsHandler->loan);
}

LoanHandlerAgg::LoanHandlerAgg(
    std::unique_ptr<ParamsHandler>&& pParamsHandler,
    std::unique_ptr<AcceleratedPay::Params> pParamsAccPay,
    std::unique_ptr<LoanPayoff::Params> pParamsPayoff)
    : _pParamsHandler{std::move(pParamsHandler)}, _accPay{std::move(pParamsAccPay)}, _payoff{std::move(pParamsPayoff)} {}

PaymentBreakdown LoanHandlerAgg::processMonth(tUint nMonthsDone, tAmount amountSpare) {
    auto& loan = _pParamsHandler->loan;
    auto& settings = _pParamsHandler->settings;

    if (_accPay.isAnnualReset(nMonthsDone)) {
        _accPay.annualReset(loan.amount, loan.amountInit);
    }

    PaymentBreakdown result = processMonthNormal(settings, nMonthsDone, loan);

    amountSpare -= (result.interest + result.principle);
    tAmount amountPossible = _accPay.amountUnschedPossible(amountSpare, nMonthsDone);
    if (amountPossible > 0) {
        result.specialRepayment = _accPay.processUnschedPayment(loan, amountPossible);
    }

    return result;
}

tAmount LoanHandlerAgg::processRepayment(tAmount amountRepayment) {
    LoanStateChange stateChange;
    stateChange.principle = amountRepayment;

    _pParamsHandler->loan.processStateChange(stateChange);

    return stateChange.principle;
}

tAmount LoanHandlerAgg::processRepayment() {
    return processRepayment_imp(_pParamsHandler->loan);
}

LoanStateChange LoanHandlerAgg::processPayOffOption(tUint nMonthsDone, tAmount amountSpare, tUint nMonthsLoan) {
    LoanStateChange stateChange;
    if (amountSpare <= 0.0) {
        return stateChange;
    }

    auto& params = *_pParamsHandler;
    auto penaltyInfo = _payoff.payOffPenalty(params.loan.amount, nMonthsDone, nMonthsLoan, params.settings.interestRatesAllMonths[nMonthsDone]);

    if ((penaltyInfo.amountSaved > 0) && (amountSpare >= penaltyInfo.amount + params.loan.amount)) {
        stateChange = _payoff.processPayOff(params.loan, penaltyInfo.amount);
    }

    return stateChange;
}

void LoanHandlerAgg::makeCheckPoint() {
    _pParamsHandler->loan.makeCheckPoint();
    _accPay.makeCheckPoint();
}

void LoanHandlerAgg::resetToCheckPoint() {
    _pParamsHandler->loan.resetToCheckPoint();
    _accPay.resetToCheckPoint();
}

LoanHandlerConsv::LoanHandlerConsv(std::unique_ptr<ParamsHandler>&& pParamsHandler, std::unique_ptr<LoanHoliday::Params>&& pParamsHoliday)
    : _pParamsHandler{std::move(pParamsHandler)}, _loanHoliday{std::move(pParamsHoliday)} {}

PaymentBreakdown LoanHandlerConsv::processMonth(tUint nMonthsDone) {
    PaymentBreakdown result;

    auto& loan = _pParamsHandler->loan;
    auto& settings = _pParamsHandler->settings;

    if (_loanHoliday.isAnnualReset(nMonthsDone)) {
        _loanHoliday.annualReset();
    }

    if (_loanHoliday.isHolidayPossible()) {
        result.interest = processHoliday(settings.interestRatesAllMonths[nMonthsDone], settings.monthlyPaymentMax, loan, _loanHoliday);
    } else {
        result = processMonthNormal(settings, nMonthsDone, loan);
        _loanHoliday.resetSequence();
    }

    return result;
}

tAmount LoanHandlerConsv::processRepayment() {
    return processRepayment_imp(_pParamsHandler->loan);
}

void LoanHandlerConsv::makeCheckPoint() {
    _pParamsHandler->loan.makeCheckPoint();
    _loanHoliday.makeCheckPoint();
}

void LoanHandlerConsv::resetToCheckPoint() {
    _pParamsHandler->loan.resetToCheckPoint();
    _loanHoliday.resetToCheckPoint();
}

PaymentBreakdown LoanHandlerNor::processFirstMonth(tFloat fracInterest, tFloat fracRepayment) {
    auto& paramsHandler = *_pParamsHandler;
    return processFirstMonth_imp(paramsHandler.settings, paramsHandler.loan, fracInterest, fracRepayment);
}
PaymentBreakdown LoanHandlerAgg::processFirstMonth(tFloat fracInterest, tFloat fracRepayment) {
    auto& paramsHandler = *_pParamsHandler;
    return processFirstMonth_imp(paramsHandler.settings, paramsHandler.loan, fracInterest, fracRepayment);
}
PaymentBreakdown LoanHandlerConsv::processFirstMonth(tFloat fracInterest, tFloat fracRepayment) {
    auto& paramsHandler = *_pParamsHandler;
    return processFirstMonth_imp(paramsHandler.settings, paramsHandler.loan, fracInterest, fracRepayment);
}

} // namespace loan
} // namespace fisim

namespace {

void validatePayment(tAmount monthlyPaymentCap, LoanStateChange& payment) {
    auto const paymentTotal = payment.interest + payment.principle;

    if (paymentTotal >= monthlyPaymentCap) {                      // the monthly cap should be paid
        payment.principle = monthlyPaymentCap - payment.interest; // possibly negative amortization

        if (payment.interest >= monthlyPaymentCap) {
            payment.interest = monthlyPaymentCap;
        }
    }
}

PaymentBreakdown processMonthNormal(LoanSettings const& settings, tUint nMonthsDone, Loan& loan) {
    PaymentBreakdown result;
    if (loan.amount == 0.0) {
        return result;
    }

    LoanStateChange stateChange;
    stateChange.interest = loan.amount * settings.interestRatesAllMonths[nMonthsDone];

    switch (settings.repaymentOption) {
        case tRepaymentOption::monthlyPayment: {
            if (stateChange.interest < settings.monthlyPayment) {
                stateChange.principle = settings.monthlyPayment - stateChange.interest;
            }

            break;
        }
        default: { // interest-only, principle is already zero
        }
    }

    validatePayment(settings.monthlyPaymentMax, stateChange); // here negative amortization is treated

    loan.processStateChange(stateChange);
    result.interest = stateChange.interest;
    result.principle = stateChange.principle;

    return result;
}

PaymentBreakdown processFirstMonth_imp(LoanSettings const& settings, Loan& loan, tFloat fracInterest, tFloat fracRepayment) {
    assert(fracInterest >= 0 && fracInterest <= 1);
    assert(fracRepayment >= 0 && fracRepayment <= 1);

    PaymentBreakdown result = processMonthNormal(settings, 0u, loan);

    // put back the difference into the loan
    LoanStateChange stateChange;
    stateChange.interest = result.interest * fracInterest - result.interest;
    stateChange.principle = result.principle * fracRepayment - result.principle;
    loan.processStateChange(stateChange); // negatives

    result.interest += stateChange.interest;
    result.principle += stateChange.principle;
    result.specialRepayment = -loan.amountInit; // we issue the loan amount

    return result;
}

tAmount processRepayment_imp(Loan& loan) {
    tAmount principle = loan.amount;
    loan.amount = 0.0;

    return principle;
}

tAmount processHoliday(tFloat interestRate, tAmount monthlyPaymentCap, Loan& loan, LoanHoliday& loanHoliday) {
    LoanStateChange stateChange;
    stateChange.interest = loan.amount * interestRate;

    validatePayment(monthlyPaymentCap, stateChange);
    loan.processStateChange(stateChange);

    loanHoliday.apply();
    return stateChange.interest;
}

} // namespace

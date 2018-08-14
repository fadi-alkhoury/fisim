#include "loan_controller.h"
#include <cassert>
#include <limits>

namespace {
using tUint = fisim::tUint;
using tInt = fisim::tInt;
using tFloat = fisim::tFloat;
using LoanResult = fisim::loan::LoanController::LoanResult;
using ParamsController = fisim::loan::LoanController::ParamsController;

template <typename T> LoanResult processMonth_imp(tUint nMonthsDone, ParamsController const* pParams, T& loanHandler);

} // namespace

namespace fisim {
namespace loan {

LoanControllerNor::LoanControllerNor(std::unique_ptr<ParamsController>&& pParamsController, std::unique_ptr<ParamsHandler>&& pParamsHandler)
    : _pParams{std::move(pParamsController)}, _loanHandler{std::move(pParamsHandler)} {
    assert(static_cast<tInt>(_pParams->nMonthsToEnd) > _pParams->nMonthsOffset);
}

LoanResult LoanControllerNor::processMonth(tUint nMonthsDone, tAmount amountSpare) {
    static_cast<void>(amountSpare); // unused
    assert(!isPaidOff());

    return processMonth_imp(nMonthsDone, _pParams.get(), _loanHandler);
}

LoanControllerAgg::LoanControllerAgg(
    std::unique_ptr<ParamsController>&& pParamsController,
    std::unique_ptr<ParamsHandler>&& pParamsHandler,
    std::unique_ptr<AcceleratedPay::Params>&& pParamsAccPay,
    std::unique_ptr<LoanPayoff::Params>&& pParamsPayoff,
    std::vector<Bool>&& monthsAllowed)
    : _pParams{std::move(pParamsController)}, _loanHandler{std::move(pParamsHandler), std::move(pParamsAccPay), std::move(pParamsPayoff)}, _monthsAllowed{
                                                                                                                                               std::move(monthsAllowed)} {
    assert(static_cast<tInt>(_pParams->nMonthsToEnd) > _pParams->nMonthsOffset);
}

LoanResult LoanControllerAgg::processMonth(tUint nMonthsDone, tAmount amountSpare) {
    assert(!isPaidOff());
    LoanResult result;
    ParamsController const& params = *_pParams;
    result.pId = &params.id;

    tInt nMonthsDoneSigned = static_cast<tInt>(nMonthsDone);
    if (nMonthsDoneSigned < params.nMonthsOffset) {
        // do nothing
    } else if (nMonthsDoneSigned == params.nMonthsOffset) {
        result.paymentDue = _loanHandler.processFirstMonth(params.fracFirstMonthInterest, params.fracFirstMonthRepayment);
    } else {
        if (!_monthsAllowed[nMonthsDone]) { // deactivate aggressive options if not allowed this month
            amountSpare = 0.0;
        }

        if (nMonthsDone == params.nMonthsToEnd - 1u) { // last month
            // pay the normal payment, with no extra
            result.paymentDue = _loanHandler.processMonth(nMonthsDone);

            if (params.isForceRepay) { // make a full repayment
                result.paymentDue.specialRepayment = _loanHandler.processRepayment();
            } else { // repay as much as possible
                amountSpare -= (result.paymentDue.interest + result.paymentDue.principle);
                if (amountSpare > 0.0) {
                    result.paymentDue.specialRepayment = _loanHandler.processRepayment(amountSpare);
                }
            }
        } else {
            // try to payoff
            auto const nMonthsToEnd = nMonthsDone < params.nMonthsToEnd ? params.nMonthsToEnd : std::numeric_limits<tUint>::max();
            Loan::LoanStateChange const stateChange = _loanHandler.processPayOffOption(nMonthsDone, amountSpare, nMonthsToEnd);

            if (stateChange.principle != 0.0) { // payoff made
                result.paymentDue.specialRepayment = stateChange.principle;
                result.paymentDue.interest = stateChange.interest;
            } else { // payoff not done. just do the month with a possible extra
                result.paymentDue = _loanHandler.processMonth(nMonthsDone, amountSpare);
            }
        }
    }

    return result;
}

LoanControllerConsv::LoanControllerConsv(
    std::unique_ptr<ParamsController>&& pParamsController,
    std::unique_ptr<ParamsHandler>&& pParamsHandler,
    std::unique_ptr<LoanHoliday::Params>&& pParamsHoliday)
    : _pParams{std::move(pParamsController)}, _loanHandler{std::move(pParamsHandler), std::move(pParamsHoliday)} {
    assert(static_cast<tInt>(_pParams->nMonthsToEnd) > _pParams->nMonthsOffset);
}

LoanResult LoanControllerConsv::processMonth(tUint nMonthsDone, tAmount amountSpare) {
    static_cast<void>(amountSpare); // unused
    assert(!isPaidOff());

    return processMonth_imp(nMonthsDone, _pParams.get(), _loanHandler);
}

} // namespace loan
} // namespace fisim

namespace {

template <typename T> LoanResult processMonth_imp(tUint nMonthsDone, ParamsController const* pParams, T& loanHandler) {
    LoanResult result;
    ParamsController const& params = *pParams;

    result.pId = &params.id;
    tInt nMonthsDoneSigned = static_cast<tInt>(nMonthsDone);
    if (nMonthsDoneSigned < params.nMonthsOffset) {
        // do nothing
    } else if (nMonthsDoneSigned == params.nMonthsOffset) {
        result.paymentDue = loanHandler.processFirstMonth(params.fracFirstMonthInterest, params.fracFirstMonthRepayment);
    } else {
        result.paymentDue = loanHandler.processMonth(nMonthsDone);

        if (nMonthsDone == params.nMonthsToEnd - 1u && params.isForceRepay) { // last month and no refinancing
            result.paymentDue.specialRepayment = loanHandler.processRepayment();
        }
    }

    return result;
}
} // namespace

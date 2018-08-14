#include "tax_account.h"
#include "utils_math.h"
#include <cassert>

namespace fisim {
namespace tax {

void TaxAccount::configure(std::unique_ptr<Config>&& pConfig, CalendarMonth monthInit) {
    _pConfig = std::move(pConfig);

    CalendarMonth const taxYearStartMonth = _pConfig->taxYearStartMonth;
    if (taxYearStartMonth != CalendarMonth::none) {
        // validate
        bool const isTaxYearStart = monthInit == taxYearStartMonth;

        const tUint normalizedSimStartMonth = calMonthsDiff(taxYearStartMonth, monthInit);                 // number of months from start of tax year to sim start
        const tUint normalizedTaxReturnMonth = calMonthsDiff(taxYearStartMonth, _pConfig->taxReturnMonth); // number of months from start of tax year to tax return
        bool const isTaxReturnOver = (normalizedTaxReturnMonth < normalizedSimStartMonth);

        if (isTaxReturnOver || isTaxYearStart) { // expect palceholders
            assert(std::isnan(state.amountTaxReturn));
            assert(std::isnan(state.taxRate));
        } else {
            assert(!std::isnan(state.amountTaxReturn));
            assert(!std::isnan(state.taxRate));
        }

        // if starting in a new tax year, the tax return and new taxrate are updated
        if (isTaxYearStart) {
            processState(0);
        }

        // compute the next tax return timestep
        if (isTaxReturnOver) {
            state.nMonthTaxReturn = (normalizedTaxReturnMonth + 12U) - normalizedSimStartMonth;
        } else {
            state.nMonthTaxReturn = normalizedTaxReturnMonth - normalizedSimStartMonth;
        }
    } else {                                                         // fixed tax account
        assert(state.nMonthTaxReturn != constants::limitMax<tUint>); // fixed settlement date should be provided,
        assert(std::isnan(state.amountTaxReturn));                   // the tax return is intended to be computed with processState() as
                                                                     // transactions are made
        assert(std::isnan(state.taxRate));
    }
}

void TaxAccount::processState(tInt nMonthNow) {
    auto& config = *_pConfig;

    bool const isFixedAccount = (config.taxYearStartMonth == CalendarMonth::none); // earnings, taxPaid and deductions don't get zeroed out

    tAmount const deductionClaimable = (state.deductions > state.earnings) ? state.earnings : state.deductions;
    if (!isFixedAccount) {
        state.deductions -= deductionClaimable;
        if (state.deductions > 0.0 && !config.isTaxLossCarriedForward) {
            state.deductions = 0.0;
        }
    }

    tAmount taxableIncome = state.earnings - deductionClaimable - config.amountDeductible.next();
    taxableIncome = taxableIncome < 0.0 ? 0.0 : taxableIncome; // no negative income tax

    auto const& dataPoints = config.progTaxDataPoints;
    state.taxRate = utils::interpolate(taxableIncome, dataPoints.incomeAmounts, dataPoints.taxRates);

    tAmount const amountTaxRequired = taxableIncome * state.taxRate;
    state.amountTaxReturn = state.taxPaid - amountTaxRequired;

    if (!isFixedAccount && nMonthNow >= static_cast<tInt>(state.nMonthTaxReturn)) { // not a fixed account and TR date should be updated
        state.nMonthTaxReturn += 12;
        state.earnings = 0.0;
        state.taxPaid = 0.0;
    }
}

void TaxAccount::makeCheckPoint() {
    _stateCheckPoint = state;
}

void TaxAccount::resetToCheckPoint() {
    state = _stateCheckPoint;
}

} // namespace tax
} // namespace fisim

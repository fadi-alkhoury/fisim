#include "tax_handler.h"
#include <cassert>

namespace fisim {
namespace tax {

using Accounts = std::vector<fisim::tax::TaxAccount>;

TaxHandler::TaxHandler(std::vector<TaxAccount>&& accounts, CalendarMonth monthInit) : _accounts{std::move(accounts)}, _getCalMonth{monthInit} {
    auto nAccounts = _accounts.size();

    taxReturns.resize(nAccounts);
    for (auto i = 0u; i < nAccounts; ++i) {
        taxReturns[i].pId = &_accounts[i].id();
        taxReturns[i].pAmount = &_accounts[i].state.amountTaxReturn;
        taxReturns[i].pNmonth = &_accounts[i].state.nMonthTaxReturn;

        _accountIndexMap[_accounts[i].id().idTax] = i;
    }
}

tAmount TaxHandler::processTax(tInt idTax, tAmount amount, tUint nMonthNow, tFloat rate, bool isAppliedAtSource) {
    const tSize idx = _accountIndexMap.at(idTax); // Throws if the account doesn't exist

    tAmount taxDue = 0.0;
    bool isIncoming = (amount >= 0.0);
    if (isIncoming) {
        _accounts[idx].state.earnings += amount;

        if (isAppliedAtSource) {
            taxDue = amount * rate;
            _accounts[idx].state.taxPaid += taxDue;
        }
    } else {
        tAmount amountEffective = amount * rate;            // rate is the proportion of the amount that is deductible
        _accounts[idx].state.deductions -= amountEffective; // adding the positive value
        taxDue = 0.0;
    }

    // for a fixed account, process the state so that TR is updated
    if (_accounts[idx].taxYearStartMonth() == CalendarMonth::none) {
        _accounts[idx].processState(nMonthNow);
    }

    return taxDue;
}

void TaxHandler::processDeduction(tInt idTax, tAmount amount) {
    assert(amount >= 0);

    const tSize idx = _accountIndexMap.at(idTax); // Throws if the account doesn't exist
    _accounts[idx].state.deductions += amount;
}

void TaxHandler::finishMonth(tUint nMonthNow) {
    CalendarMonth taxYearEnd;
    for (auto& account : _accounts) {
        CalendarMonth const taxYearStartMonth = account.taxYearStartMonth();
        if (taxYearStartMonth != CalendarMonth::none) { // not a fixed account
            taxYearEnd = incrementMonth(taxYearStartMonth, -1);
            if (_getCalMonth(nMonthNow) == taxYearEnd) {
                account.processState(nMonthNow);
            }
        }
    }
}

TaxAccount const& TaxHandler::account(tInt idTax) const {
    const tSize idx = _accountIndexMap.at(idTax); // Throws if the account doesn't exist
    return _accounts[idx];
}

void TaxHandler::makeCheckPoint() {
    for (auto& account : _accounts) {
        account.makeCheckPoint();
    }
}

void TaxHandler::resetToCheckPoint() {
    for (auto& account : _accounts) {
        account.resetToCheckPoint();
    }
}

} // namespace tax
} // namespace fisim

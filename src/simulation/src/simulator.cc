#include "simulator.h"
#include "utils_math.h"

#include <cassert>
#include <stdexcept>

using MonthlyRecords = std::vector<fisim::transacs::TransacsRecorder::MonthTransacs>;
using fisim::utils::isAlmostEqual;

namespace fisim {
namespace simulation {

Simulator::Simulator(SimComponents&& simComponents)
    : _cashflows{std::move(simComponents.cashflows)}, _loanControllers{std::move(simComponents.loanControllers)}, _taxHandler{std::move(simComponents.taxHandler)} {}

void Simulator::simulate(tUint nMonths, tAmount balance, tAmount balanceReserve) {
    _transacsRecorder.init(nMonths, balance);
    _balanceReserve = balanceReserve;

    for (tUint nMonthNow = 0u; nMonthNow < nMonths; nMonthNow++) {
        _transacsRecorder.startMonth();

        simulateMonth(nMonthNow);

        if (_transacsRecorder.balance() < balanceReserve - constants::tolerance) {
            throw MinBalanceViolation{};
        }
    }
}

void Simulator::simulateMonth(tUint nMonthNow) {
    // process cashflows
    for (auto& cashflow : _cashflows) {
        auto result = cashflow.next();
        Id id = *result.pId;

        if (!isAlmostEqual(result.amount, 0.0)) { // it's 0 if the cashflow is inactive
            tAmount amountEff = result.amount - result.amountLost;

            tInt idTax = id.idTax;
            id.idTax = 0; // 0 for non-tax transacs
            _transacsRecorder.recordTransac(id, result.amount);
            if (result.amountLost != 0.0) {
                _transacsRecorder.recordTransac(id, -result.amountLost, "loss");
            }
            id.idTax = idTax; // change back to what it was

            tAmount taxDue = 0.0;
            if (id.idTax != 0) {
                taxDue = _taxHandler.processTax(id.idTax, amountEff, nMonthNow, result.taxFactor, result.shouldApplyAtSource);
            }

            if (!isAlmostEqual(taxDue, 0.0)) {
                _transacsRecorder.recordTransac(id, -taxDue);
            }
        }
    }

    // process any tax returns
    for (auto const& update : _taxHandler.taxReturns) {
        if (nMonthNow == *(update.pNmonth)) {
            tAmount amountReturn = *update.pAmount;
            if (!isAlmostEqual(amountReturn, 0.0)) {
                _transacsRecorder.recordTransac(*update.pId, amountReturn);
            }
        }
    }

    // process loan payments
    //   Note: a negative balance could result from an aggressive repayment strategy since we don't foresee the future
    //   state when executing the strategy.
    for (auto& pLoanController : _loanControllers) {
        auto& loanController = *pLoanController;

        if (!loanController.isPaidOff()) {
            auto amountSpare = _transacsRecorder.balance() - _balanceReserve;
            auto loanResult = loanController.processMonth(nMonthNow, amountSpare);

            Id const& id = *loanResult.pId;
            if (loanResult.paymentDue.interest > 0.0) {
                _transacsRecorder.recordTransac(id, -loanResult.paymentDue.interest, "interest");
            }
            if (loanResult.paymentDue.principle > 0.0) {
                _transacsRecorder.recordTransac(id, -loanResult.paymentDue.principle, "repayment");
            }
            if (loanResult.paymentDue.specialRepayment > 0.0) {
                _transacsRecorder.recordTransac(id, -loanResult.paymentDue.specialRepayment, "extra_repayment");
            }

            if (id.isTaxDeductibleInterest()) {
                _taxHandler.processDeduction(id.idTax, loanResult.paymentDue.interest);
            }
        }
    }

    _taxHandler.finishMonth(nMonthNow);
}

void Simulator::makeCheckPoint() {
    _transacsRecorder.makeCheckPoint();

    for (auto& cashflow : _cashflows) {
        cashflow.makeCheckPoint();
    }
    for (auto& pLoanController : _loanControllers) {
        pLoanController->makeCheckPoint();
    }
    _taxHandler.makeCheckPoint();
}

void Simulator::resetToCheckPoint() {
    _transacsRecorder.resetToCheckPoint();

    for (auto& cashflow : _cashflows) {
        cashflow.resetToCheckPoint();
    }
    for (auto& pLoanController : _loanControllers) {
        pLoanController->resetToCheckPoint();
    }
    _taxHandler.resetToCheckPoint();
}

} // namespace simulation
} // namespace fisim

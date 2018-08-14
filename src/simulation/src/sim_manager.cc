#include "sim_manager.h"
#include "binary_int_solver.h"
#include "utils_data.h"
#include <cassert>
#include <stdexcept>

namespace {
using tSize = fisim::tSize;
using tUint = fisim::tUint;
using tInt = fisim::tInt;
using tAmount = fisim::tAmount;
using MonthlyRecords = fisim::transacs::TransacsRecorder::MonthlyRecords;
using Simulator = fisim::simulation::Simulator;
using SimBackdoor = fisim::simulation::SimManager::SimBackdoor;

struct TransacPosition {
    TransacPosition(tSize indMonth, tSize indTransac) : indMonth{indMonth}, indTransac{indTransac} {}
    tSize indMonth;
    tSize indTransac;
};

struct TransacsPositionsAndAmounts {
    std::vector<TransacPosition> positions;
    std::vector<tAmount> amounts;
};

// returns inds and positive amounts of avoidable transacs
TransacsPositionsAndAmounts findAvoidableTransacs(MonthlyRecords const& monthlyRecords, SimBackdoor const& simBackdoor);

void setupNextSim(MonthlyRecords const& monthlyRecords, std::vector<TransacPosition> const& positionsToAvoid, SimBackdoor& simBackdoor);

bool handleMinBalanceViolation(tUint nMonths, tAmount balance, tAmount balanceReserve, Simulator& simulator, SimBackdoor& simBackdoor);

tSize findIndLoan(tInt idLoanTarget, std::vector<SimBackdoor::LoanBackdoor> const& loansBackdoors);

} // namespace

namespace fisim {
namespace simulation {

SimManager::SimManager(SimBackdoor&& simBackdoor, Simulator& simulator) : _simBackdoor{std::move(simBackdoor)}, _simulator{simulator} {}

void SimManager::simulate(tUint nMonths, tAmount balance, tAmount balanceReserve) {
    _simulator.makeCheckPoint();

    for (tUint nTries = 0u; nTries < 100u; nTries++) {
        try {
            _simulator.simulate(nMonths, balance, balanceReserve);
            break;
        } catch (Simulator::MinBalanceViolation const& e) {
            if (nTries != 99u) { // we can retry with different params
                bool shouldRetry = handleMinBalanceViolation(nMonths, balance, balanceReserve, _simulator, _simBackdoor);
                if (!shouldRetry)
                    break;
            }
        }
    }
}

} // namespace simulation
} // namespace fisim

namespace {

bool handleMinBalanceViolation(tUint nMonths, tAmount balance, tAmount balanceReserve, Simulator& simulator, SimBackdoor& simBackdoor) {
    auto& monthlyRecords = simulator.monthlyRecords();

    auto avoidableTransacs = findAvoidableTransacs(monthlyRecords, simBackdoor);
    auto nAvoidableTransacs = avoidableTransacs.amounts.size();

    if (nAvoidableTransacs > 0u) { // some transacs can be avoided, prepare the next sim
        auto nMonthsDone = monthlyRecords.size();
        tAmount amountNeeded = -monthlyRecords[nMonthsDone - 1u].balance + 0.01;
        std::vector<tSize> combination = fisim::utils::findFeasibleComb(avoidableTransacs.amounts, amountNeeded);

        auto nTransacsChosen = combination.size();
        if (nTransacsChosen > 0u) { // feasible combination found
            std::vector<TransacPosition> positionsToAvoid;
            positionsToAvoid.reserve(nTransacsChosen);

            for (auto ind : combination) {
                positionsToAvoid.push_back(avoidableTransacs.positions[ind]);
            }

            simulator.resetToCheckPoint();
            setupNextSim(monthlyRecords, positionsToAvoid, simBackdoor);
        } else { // feasible combination not found! avoid all the avoidables, rerun and return
            simulator.resetToCheckPoint();
            setupNextSim(monthlyRecords, avoidableTransacs.positions, simBackdoor);

            try {
                simulator.simulate(nMonths, balance, balanceReserve);
            } catch (Simulator::MinBalanceViolation const& e) {
            }

            return false; // should not retry
        }
    } else {          // no avoidable transacs, simply report the sim result
        return false; // should not retry
    }

    return true; // should retry
}

TransacsPositionsAndAmounts findAvoidableTransacs(MonthlyRecords const& monthlyRecords, SimBackdoor const& simBackdoor) {
    TransacsPositionsAndAmounts avoidableTransacs;
    avoidableTransacs.positions = fisim::utils::makeReservedVector<TransacPosition>(50u);
    avoidableTransacs.amounts = fisim::utils::makeReservedVector<tAmount>(50u);

    auto nMonths = monthlyRecords.size();
    for (auto indMonth = 0u; indMonth < nMonths; indMonth++) {
        auto const& transacs = monthlyRecords[indMonth].transacs;
        auto nTransacs = transacs.size();

        for (auto indTransac = 0u; indTransac < nTransacs; indTransac++) {
            if (transacs[indTransac].id.isLoan()) {
                // There exists 3 transacs, and the third one is avoidable (if non zero)
                indTransac += 2;
                tAmount const amount = transacs[indTransac].amount;

                if (amount < 0.0) {
                    tSize indLoan = findIndLoan(transacs[indTransac].id.idLoan, simBackdoor.loansBackdoors);
                    auto const& paramsController = *(simBackdoor.loansBackdoors[indLoan].pParamsController);

                    bool const isForceRepayDone = (indMonth == paramsController.nMonthsToEnd - 1) && paramsController.isForceRepay;

                    if (!isForceRepayDone) {
                        avoidableTransacs.positions.emplace_back(indMonth, indTransac);
                        avoidableTransacs.amounts.push_back(-amount);
                    }
                }
            }
        } // for
    }     // for

    return avoidableTransacs;
}

void setupNextSim(MonthlyRecords const& monthlyRecords, std::vector<TransacPosition> const& positionsToAvoid, SimBackdoor& simBackdoor) {

    for (auto const& position : positionsToAvoid) {
        auto const& transac = monthlyRecords[position.indMonth].transacs[position.indTransac];

        if (transac.id.isLoan()) {
            fisim::Bool* pMonthsAggAllowed = simBackdoor.loansBackdoors[transac.id.idLoan - 1u].pMonthsAggAllowed;
            assert(pMonthsAggAllowed != nullptr);

            pMonthsAggAllowed[position.indMonth] = false;
        } else {
            assert(false); // avoidable transacs come only from loans atm
        }
    }
}

tSize findIndLoan(tInt idLoanTarget, std::vector<SimBackdoor::LoanBackdoor> const& loansBackdoors) {
    tSize nLoans = loansBackdoors.size();
    tSize indFound = nLoans;

    for (tSize i = 0u; i < nLoans; ++i) {
        if (loansBackdoors[i].pParamsController->id.idLoan == idLoanTarget) {
            indFound = i;
            break;
        }
    }
    assert(indFound != nLoans);

    return indFound;
}

} // namespace

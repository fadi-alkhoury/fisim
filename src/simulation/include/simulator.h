#ifndef FISIM_SIMULATION_H_
#define FISIM_SIMULATION_H_

#include "cashflow.h"
#include "loan_controller.h"
#include "tax_handler.h"
#include "transacs_recorder.h"
#include "types.h"

#include <stdexcept>

namespace fisim {
namespace simulation {

class Simulator final {
  public:
    struct SimComponents {
        std::vector<cashflow::Cashflow> cashflows;
        std::vector<std::unique_ptr<loan::LoanController>> loanControllers;
        tax::TaxHandler taxHandler;
    };

    struct MinBalanceViolation : public std::logic_error {
        MinBalanceViolation() : std::logic_error{"Insufficient balance to continue the simulation."} {}
    };

    Simulator(SimComponents&& simComponents);
    Simulator(Simulator const&) = delete;
    Simulator(Simulator&&) = delete;

    ///
    /// Runs the simulation.
    ///
    /// \param[in] nMonths         Number of simulation months.
    /// \param[in] balance         Starting balance.
    /// \param[in] balanceReserve  Minimum balance requirement.
    ///
    void simulate(
        tUint nMonths,         ///< The number of months to simulate
        tAmount balance,       ///< The initial balance
        tAmount balanceReserve ///< The reserve balance. An exception will be thrown if the balance during the simulation falls below this value.
    );

    using MonthlyRecords = transacs::TransacsRecorder::MonthlyRecords;
    MonthlyRecords const& monthlyRecords() const { ///< the simulation result can be accessed via this function
        return _transacsRecorder.monthlyRecords();
    }

    ///
    /// Transfers ownership to the results to the caller.
    ///
    MonthlyRecords releaseRecords() { return _transacsRecorder.release(); }

    void makeCheckPoint(); ///< Makes a checkpoint of the state

    void resetToCheckPoint(); ///< resets to the previously made checkpoint

  private:
    void simulateMonth(tUint nMonthNow);

    std::vector<cashflow::Cashflow> _cashflows;
    std::vector<std::unique_ptr<loan::LoanController>> _loanControllers;
    tax::TaxHandler _taxHandler;

    transacs::TransacsRecorder _transacsRecorder;
    tAmount _balanceReserve = 0.0;
};

} // namespace simulation
} // namespace fisim

#endif /* FISIM_SIMULATION_H_ */

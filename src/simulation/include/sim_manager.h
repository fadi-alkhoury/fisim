#ifndef FISIM_SIM_MANAGER_H_
#define FISIM_SIM_MANAGER_H_

#include "simulator.h"
#include "types.h"

namespace fisim {
namespace simulation {

class SimManager final {
  public:
    ///
    /// Provides access to simulator configuration.
    ///
    struct SimBackdoor {
        struct LoanBackdoor {
            loan::LoanController::ParamsController const* pParamsController;
            Bool* pMonthsAggAllowed = nullptr; ///< pointer to monthsAllowed of LoanControllerAgg
        };

        std::vector<LoanBackdoor> loansBackdoors;
    };

    ///
    /// Constructs a simulation manager, which uses the simulator to run simulation, and retries simulations if needed.
    ///
    /// \param[in]     simBackdoor  Provides access to simulator configuration.
    /// \param[in,out] simulator    The simulator to be managed.
    ///
    SimManager(SimBackdoor&& simBackdoor, Simulator& simulator);

    SimManager(SimManager const&) = delete;
    SimManager(SimManager&&) = delete;

    ///
    /// Runs the simulation. Simulations are retried with different settings in case the balance falls below the reserve requirement.
    ///
    /// \param[in] nMonths         Number of simulation months.
    /// \param[in] balance         Starting balance.
    /// \param[in] balanceReserve  Minimum balance requirement.
    ///
    void simulate(tUint nMonths, tAmount balance, tAmount balanceReserve);

  private:
    SimBackdoor _simBackdoor;
    Simulator& _simulator;
};

} // namespace simulation
} // namespace fisim

#endif /* FISIM_SIM_MANAGER_H_ */

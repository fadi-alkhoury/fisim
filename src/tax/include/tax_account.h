#ifndef FISIM_TAX_TAX_ACCOUNT_H_
#define FISIM_TAX_TAX_ACCOUNT_H_

#include "types.h"
#include "uncertain_amount.h"
#include <memory>
#include <vector>

namespace fisim {
namespace tax {

///
/// The tax account. First needs to be default initialized, then the state has to be set, and then configure should be called
///
class TaxAccount final {
  public:
    struct State {
        /// Initialization
        ///	  The earnings, deductions, taxPaid always need to be initalized manually, even if starting in a
        ///   new year (then the values are used to calculate the tax return for the completed year)
        ///
        ///   Fixed tax account:
        ///     Does not have the periodic tax year. All values are needed except amountTaxReturn and taxRate.
        ///
        ///   Normal tax account:
        /// 	let Ts: start month, Tr: tax return month normalized by Ts, Ty: tax year start month normalized by Ts
        /// 	amountTaxReturn and taxRate should be manually initialized only if
        /// 		Ty < Ts <= Tr
        /// 	If this condition is not met, they should not be initialized manually.
        /// 	nMonthTaxReturn should not be initialized manually in any case.
        ///
        tAmount earnings;
        tAmount deductions;
        tAmount taxPaid;
        tAmount amountTaxReturn = constants::NaN<tAmount>;
        tFloat taxRate = constants::NaN<tFloat>;
        tUint nMonthTaxReturn = constants::limitMax<tUint>; ///< simulation month where the next tax return should be paid
    };

    struct Config {
        struct ProgTaxSpec {
            std::vector<tAmount> incomeAmounts; ///< annual income amounts
            std::vector<tFloat> taxRates;       ///< The portion of the incomeAmounts that go towards taxes and mandatory contributions.
        };

        Id id;
        CalendarMonth taxYearStartMonth; ///< The month from which the tax year starts, OR, CalendarMonth::none FOR A FIXED ACCOUNT (no tax year)
        bool isTaxLossCarriedForward;    ///< carry unused deductions forward to next years. IRRELEVENT FOR FIXED ACCOUNTS
        CalendarMonth taxReturnMonth;    ///< the tax return from the previous tax year is payed on this month. IRRELEVENT FOR FIXED ACCOUNTS
        ProgTaxSpec progTaxDataPoints;
        UncertainAmount amountDeductible; ///< tax deductible amount that applies for a tax year each year
    };

    void configure(std::unique_ptr<Config>&& pConfig, CalendarMonth monthInit); ///< Needs to be called after setting the state

    void processState(tInt nMonthNow);

    void makeCheckPoint();

    void resetToCheckPoint();

    Id const& id() const { return _pConfig->id; }
    CalendarMonth taxYearStartMonth() const { return _pConfig->taxYearStartMonth; }

    State state;

  private:
    std::unique_ptr<Config> _pConfig;

    State _stateCheckPoint;
};

} // namespace tax
} // namespace fisim

#endif /* FISIM_TAX_TAX_ACCOUNT_H_ */

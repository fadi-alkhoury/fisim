#ifndef SRC_CASHFLOW_CASHFLOW_H_
#define SRC_CASHFLOW_CASHFLOW_H_

#include "types.h"
#include "uncertain_amount.h"

#include <vector>
#include <memory>

namespace fisim{
namespace cashflow {

class Cashflow final {
public:
	struct CashflowResult {
		Id const* pId;                     ///< ID of the cashflow
		tAmount amount = 0.0;              ///< The cashfow amount if there are is no loss
		tAmount amountLost = 0.0;          ///< The amount that is lost
		tFloat taxFactor = 0.0;            ///< +ive cashflow -> taxrate, -ive cashflow -> tax deductible
		bool shouldApplyAtSource = false;  ///< Whether tax should be applied immediately or with the tax return
	};

	struct TaxInfo {
		struct NextUpdate {
			tFloat const* pRate;  ///<  Pointer to the next rate
			tUint const* pNmonth; ///< Pointer to the simulation month when the update comes to effect. null -> will never update
		};

		tFloat rate; ///< +ive cashflow -> taxrate, -ive cashflow -> tax deductible
		bool shouldApplyAtSource;
		NextUpdate nextUpdate;
	};

	struct Params {
		Id id;
		UncertainAmount uncertainAmount;
		std::vector<Bool> stepsActive;
		TaxInfo taxInfo;
		tFloat lossProbability; ///< The probability that the cashflow would be lost in any month.
	};

	explicit Cashflow(std::unique_ptr<Params>&& pParams);

	///
	/// Finds the resulting cash flow in the next step, and updates the tax rate when it's time to update
	///  - For an outgoing cash flow, the rate is not updated as it is a fixed deduction %
	///  - For an incoming cash flow, the rate is updated to the tax rate of the tax account
	///
	CashflowResult next();

	///
	/// Creates a checkpoint so that the cashflow can be reset to the current state.
	///
	void makeCheckPoint();

	///
	/// Resets the state of the cashflow to the checkpoint
	///
	void resetToCheckPoint();

private:
	struct CheckPoint {
		tUint nStepsDone = 0u;
		tFloat rate;
	};

	std::unique_ptr<Params> _pParams;

	tUint _nStepsDone = 0u;
	std::vector<Bool> _stepsLost;
	CheckPoint _checkPoint;
};

}} /* namespace cashflow */

#endif /* SRC_CASHFLOW_CASHFLOW_H_ */

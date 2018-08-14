#ifndef FISIM_UNCERTAIN_AMOUNT_H_
#define FISIM_UNCERTAIN_AMOUNT_H_

#include "types.h"
#include <memory>

namespace fisim {

///
/// An uncertain amount that grows periodically by an uncertain factor
///    amount = amount_base * change_factor,
///    where:
///      amount_base is uniformly distributed in [amountMin, amountMax]
///      change_factor[k+1] = change_factor[k] * uniform_dist(changeFactorMin, changeFactorMax)
///
class UncertainAmount final {
public:
	struct Params {
		tAmount amountMin = 0.0;   ///< lower bound for the base amount
		tAmount amountMax = 0.0;   ///< upper bound for the base amount
		tFloat changeFactorMin;    ///< lower bound for the change factor of the base amount
		tFloat changeFactorMax;    ///< upper bound for the change factor of the base amount
		tUint changeInterval;      ///< the interval of growth. For eg. 12 means the growth is once a year
		tInt intervalElapsedInit;  ///< the number of time steps that were elapsed in the interval at time step 0. A negative can be used to cause the interval to start in the future.

		bool isVariableBaseAmount() const { return amountMin != amountMax;}
		bool isVariableChangeFactor() const { return changeFactorMin != changeFactorMax;}
		bool hasPeriodicChange() const { return changeInterval != std::numeric_limits<tUint>::max();}
	};

	UncertainAmount() = default;

	///
	/// Create a non-growable, fixed amount (i.e. "certain" amount).
	///
	explicit UncertainAmount(tAmount amount);

	///
	/// Create an uncertain amount with provided settings.
	///
	explicit UncertainAmount(std::unique_ptr<Params>&& pParams);

	///
	/// Finds the uncertain amount in the next step.
	///
	tAmount next();

	///
	/// Creates a checkpoint so that the cashflow can be reset to the current state.
	///
	inline void makeCheckPoint() { _checkPoint = _state; }

	///
	/// Resets the state of the cashflow to the checkpoint
	///
	inline void resetToCheckPoint() { _state = _checkPoint; }

private:
	struct State {
		tUint nStepsDone = 0u;
		tUint nUpdates = 0u;
		tFloat changeFactor = 1.f;
	};

	std::unique_ptr<Params> _pParams;

	State _state;
	State _checkPoint;
};

}// namespace 
#endif /* FISIM_UNCERTAIN_AMOUNT_H_ */

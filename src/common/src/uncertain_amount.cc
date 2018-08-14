#include "uncertain_amount.h"
#include "utils_math.h"
#include <cassert>

namespace fisim {

UncertainAmount::UncertainAmount(tAmount amount) {
    _pParams = std::make_unique<Params>();
    auto& params = *_pParams;

    params.amountMin = amount;
    params.amountMax = amount;
    params.changeInterval = std::numeric_limits<tUint>::max();
}

UncertainAmount::UncertainAmount(std::unique_ptr<Params>&& pParams) : _pParams{std::move(pParams)} {
    assert(_pParams->changeFactorMin >= 0 && _pParams->changeFactorMin <= _pParams->changeFactorMax);
    assert(_pParams->amountMin <= _pParams->amountMax);
}

tAmount UncertainAmount::next() {
    auto const& params = *_pParams;

    // Find the next change factor
    tLong const nStepsDoneEff = static_cast<tLong>(_state.nStepsDone) + static_cast<tLong>(params.intervalElapsedInit);
    if (params.hasPeriodicChange() && nStepsDoneEff > 0) {
        tUint const nUpdates = static_cast<tUint>(nStepsDoneEff) / params.changeInterval; // integral division, rounded down

        if (nUpdates != _state.nUpdates) {
            _state.nUpdates = nUpdates;

            tFloat changeFactorNew;
            if (params.isVariableChangeFactor()) {
                changeFactorNew = utils::randomFactor(params.changeFactorMin, params.changeFactorMax);
            } else {
                changeFactorNew = params.changeFactorMin;
            }

            _state.changeFactor *= changeFactorNew;
        }
    }

    // Find the next base amount
    tAmount amount;
    if (params.isVariableBaseAmount()) {
        amount = utils::randomFactor(params.amountMin, params.amountMax);
    } else {
        amount = params.amountMin;
    }

    _state.nStepsDone++;

    return (_state.changeFactor * amount);
}

} // namespace fisim

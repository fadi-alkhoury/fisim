#include "cashflow.h"
#include "utils_math.h"

#include <cmath>

namespace fisim {
namespace cashflow {

Cashflow::Cashflow(std::unique_ptr<Params>&& pParams) : _pParams(std::move(pParams)) {
    const Params& params = *_pParams;
    tSize const nSteps = params.stepsActive.size();

    _stepsLost.reserve(nSteps);
    for (tSize i = 0u; i < nSteps; ++i) {
        if (utils::randomFactor() < params.lossProbability) {
            _stepsLost.push_back(true);
        } else {
            _stepsLost.push_back(false);
        }
    }

    _checkPoint.rate = params.taxInfo.rate;
}

Cashflow::CashflowResult Cashflow::next() {
    CashflowResult result;
    Params& params = *_pParams;
    result.pId = &params.id;

    if (params.stepsActive[_nStepsDone]) {
        result.amount = params.uncertainAmount.next();

        if (_stepsLost[_nStepsDone]) { // cashflow is lost completely
            result.amountLost = result.amount;
        }

        result.taxFactor = params.taxInfo.rate;
        result.shouldApplyAtSource = params.taxInfo.shouldApplyAtSource;

        const bool shouldUpdate = result.amount > 0.0 && params.taxInfo.nextUpdate.pNmonth != nullptr && _nStepsDone == *(params.taxInfo.nextUpdate.pNmonth);
        if (shouldUpdate) {
            params.taxInfo.rate = *(params.taxInfo.nextUpdate.pRate);
        }
    }

    ++_nStepsDone;

    return result;
}

void Cashflow::makeCheckPoint() {
    _checkPoint.nStepsDone = _nStepsDone;
    _checkPoint.rate = _pParams->taxInfo.rate;
    _pParams->uncertainAmount.makeCheckPoint();
}

void Cashflow::resetToCheckPoint() {
    _nStepsDone = _checkPoint.nStepsDone;
    _pParams->taxInfo.rate = _checkPoint.rate;
    _pParams->uncertainAmount.resetToCheckPoint();
}

} // namespace cashflow
} // namespace fisim

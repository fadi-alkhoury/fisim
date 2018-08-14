#include "loanholiday.h"
#include <cassert>

namespace fisim {
namespace loan {

LoanHoliday::LoanHoliday(std::unique_ptr<Params> pParams) : _pParams{std::move(pParams)} {
    assert(_pParams->nMonthsPerYear <= _pParams->nMonthsTermRemaining);
    assert(_pParams->nMonthsYearRemaining <= _pParams->nMonthsPerYear);
    assert(_pParams->nMonthsYearRemaining <= _pParams->nMonthsTermRemaining);
}

void LoanHoliday::apply() {
    if (_pParams->isExtendTerm) {
        *_pParams->pNumMonthsToEnd += 1;
    }

    _pParams->nMonthsTermRemaining--;
    _pParams->nMonthsYearRemaining--;
    _pParams->isSequenceStarted = true;
}

bool LoanHoliday::isHolidayPossible() const {
    if (_pParams->nMonthsYearRemaining < 1u) {
        return false;
    }

    bool isSequenceCanContinue = (_pParams->isSequenceStarted && _pParams->isAllowConsecutive);

    if (!_pParams->isSequenceStarted || isSequenceCanContinue) {
        return true;
    }

    return false;
}

bool LoanHoliday::isAnnualReset(tUint nMonthsDone) const {
    tUint nMonthsEffective = (nMonthsDone + _pParams->nMonthsStartOffset);

    bool isResetMonth = ((nMonthsEffective % 12u == 0u) && (nMonthsEffective > 0u));
    return isResetMonth;
}

void LoanHoliday::annualReset() {
    if (_pParams->nMonthsPerYear <= _pParams->nMonthsTermRemaining) {
        _pParams->nMonthsYearRemaining = _pParams->nMonthsPerYear;
    } else {
        _pParams->nMonthsYearRemaining = _pParams->nMonthsTermRemaining;
    }
}

void LoanHoliday::resetSequence() {
    _pParams->isSequenceStarted = false;
}

void LoanHoliday::makeCheckPoint() {
    _nMonthsYearRemainingCheckPoint = _pParams->nMonthsYearRemaining;
    _nMonthsTermRemainingCheckPoint = _pParams->nMonthsTermRemaining;
    _isSequenceStartedCheckPoint = _pParams->isSequenceStarted;
}

void LoanHoliday::resetToCheckPoint() {
    _pParams->nMonthsYearRemaining = _nMonthsYearRemainingCheckPoint;
    _pParams->nMonthsTermRemaining = _nMonthsTermRemainingCheckPoint;
    _pParams->isSequenceStarted = _isSequenceStartedCheckPoint;
}

} // namespace loan
} // namespace fisim

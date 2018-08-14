#include "transacs_recorder.h"

namespace fisim {
namespace transacs {

TransacsRecorder::Transac::Transac(Id id, tAmount amount, std::string description) : id{id}, amount{amount}, description{description} {}

void TransacsRecorder::init(tUint nMonths, tAmount balance) {
    _monthlyRecords.reserve(nMonths);
    _balanceInit = balance;
}

void TransacsRecorder::reserve(tUint nMonths) {
    _monthlyRecords.reserve(nMonths);
}

void TransacsRecorder::startMonth() {
    auto ind = _monthlyRecords.size();
    _monthlyRecords.emplace_back();
    _monthlyRecords[ind].transacs.reserve(50u);

    if (ind > 0u) {
        _monthlyRecords[ind].balance = _monthlyRecords[ind - 1].balance;
    } else {
        _monthlyRecords[ind].balance = _balanceInit;
    }
}

void TransacsRecorder::recordTransac(Id const& id, tAmount amount, std::string description) {
    auto& currentMonthRecord = _monthlyRecords.back();

    currentMonthRecord.transacs.emplace_back(id, amount, description);
    currentMonthRecord.balance += amount;
}

void TransacsRecorder::makeCheckPoint() {
    _checkPoint.nMonth = _monthlyRecords.size();
    if (_checkPoint.nMonth != 0u) {
        _checkPoint.balance = _monthlyRecords[_checkPoint.nMonth - 1u].balance;
        _checkPoint.nTransacs = _monthlyRecords[_checkPoint.nMonth - 1u].transacs.size();
    } else {
        _checkPoint.balance = _balanceInit;
        _checkPoint.nTransacs = 0u;
    }
}

void TransacsRecorder::resetToCheckPoint() {
    _monthlyRecords.resize(_checkPoint.nMonth);
    if (_checkPoint.nMonth != 0u) {
        _monthlyRecords[_checkPoint.nMonth - 1u].balance = _checkPoint.balance;
        _monthlyRecords[_checkPoint.nMonth - 1u].transacs.resize(_checkPoint.nTransacs);
    }
}

} // namespace transacs
} // namespace fisim

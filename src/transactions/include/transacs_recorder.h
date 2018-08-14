#ifndef FISIM_TRANSACTIONS_PROCESSOR_H_
#define FISIM_TRANSACTIONS_PROCESSOR_H_

#include "types.h"
#include <string>
#include <vector>

namespace fisim {
namespace transacs {

class TransacsRecorder final {
  public:
    struct Transac {
        Transac() = default;
        Transac(Id id, tAmount amount, std::string description);
        Id id;
        tAmount amount = 0.0;
        std::string description;
    };

    struct MonthTransacs {
        std::vector<Transac> transacs;
        tAmount balance; ///< Balance after all transactions in the month
    };

    using MonthlyRecords = std::vector<MonthTransacs>;

    void init(tUint nMonths, tAmount balance);

    void reserve(tUint nMonths);

    ///
    /// Starts a new month
    ///
    void startMonth();

    ///
    /// Records one transaction
    ///
    void recordTransac(Id const& id, tAmount amount, std::string description = "");

    ///
    /// Returns the balance after the last recorded transaction
    ///
    tAmount balance() const { return _monthlyRecords.back().balance; }

    ///
    /// Accessor for the recorded transactions
    ///
    MonthlyRecords const& monthlyRecords() const { return _monthlyRecords; }

    ///
    /// Transfers ownership to the caller
    ///
    MonthlyRecords release() { return std::move(_monthlyRecords); }

    ///
    /// Makes a checkpoint up to all completed transactions
    ///
    void makeCheckPoint();

    ///
    /// Resets to the previously made checkpoint
    ///
    void resetToCheckPoint();

  private:
    MonthlyRecords _monthlyRecords;
    tAmount _balanceInit;

    struct CheckPoint {
        tSize nMonth = 0u;
        tSize nTransacs = 0u;
        tAmount balance = 0.0;
    };
    CheckPoint _checkPoint;
};

} // namespace transacs
} // namespace fisim

#endif /* FISIM_TRANSACTIONS_PROCESSOR_H_ */

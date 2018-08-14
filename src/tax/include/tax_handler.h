#ifndef FISIM_TAX_TAXHANDLER_H_
#define FISIM_TAX_TAXHANDLER_H_

#include "tax_account.h"
#include "utils_data.h"

#include <unordered_map>

namespace fisim {
namespace tax {

class TaxHandler final {
  public:
    TaxHandler() = default;
    TaxHandler(std::vector<TaxAccount>&& accounts, CalendarMonth monthInit);

    tAmount processTax(tInt idTax, tAmount amount, tUint nMonthNow, tFloat rate, bool isAppliedAtSource);

    void processDeduction(tInt idTax, tAmount amount);

    struct TaxReturn {
        Id const* pId;          ///< The ID of the account
        tAmount const* pAmount; ///< The tax return amount
        tUint const* pNmonth;   ///< The simulation month when the tax return should be paid
    };
    std::vector<TaxReturn> taxReturns; ///< Should be used to create tax return transactions

    void finishMonth(tUint nMonthNow); ///< Must be called each month after all processTax and processDeduction calls

    const TaxAccount& account(tInt idTax) const; ///< accessor needed in json parsing

    void makeCheckPoint();

    void resetToCheckPoint();

  private:
    std::vector<TaxAccount> _accounts;
    std::unordered_map<tInt, tSize> _accountIndexMap; ///< taxId to index lookup

    utils::GetCalMonth _getCalMonth;
};

} // namespace tax
} // namespace fisim

#endif /* FISIM_TAX_TAXHANDLER_H_ */

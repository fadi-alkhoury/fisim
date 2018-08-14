#ifndef FISIM_LOAN_H_
#define FISIM_LOAN_H_

#include "types.h"

namespace fisim {
namespace loan {

class Loan final {
  public:
    struct LoanStateChange {
        tAmount principle = 0.0; ///< loan reduction amount
        tAmount interest = 0.0;  ///< interest to accumulate
    };

    void processStateChange(LoanStateChange& stateChange); ///< Enforce a limit and apply the change

    void makeCheckPoint();
    void resetToCheckPoint();

    tAmount amountInit; //< Initial loan amount
    tAmount amount;     //< remaining principle due. updated each month
    tAmount interestPaid;

  private:
    tAmount _amountCheckPoint = 0.0;
    tAmount _interestPaidCheckPoint = 0.0;
};

} // namespace loan
} // namespace fisim

#endif /* FISIM_LOAN_H_ */

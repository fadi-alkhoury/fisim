#ifndef FISIM_LOAN_CONTROLLER_H_
#define FISIM_LOAN_CONTROLLER_H_

#include "loan.h"
#include "loan_handler.h"
#include "types.h"
#include "utils_data.h"
#include <memory>

namespace fisim {
namespace loan {

class LoanController {
  public:
    struct ParamsController {
        Id id;              ///< positive taxId indicates that interest is tax deductible
        tUint nMonthsToEnd; ///< number of (absolute) simulation months from the start until the term is over. nMonthsToEnd - 1 is the last month of payments
        tInt nMonthsOffset; ///< number of (absolute) simulation months from the start until the term starts. nMonthsOffset 0 is the first month of payments. No partial first month
                            ///< treatment if negative
        tFloat fracFirstMonthInterest; ///< The first month can have partial interest and/or repayment
        tFloat fracFirstMonthRepayment;
        bool isForceRepay; ///< if true, the outstanding loan balance will be repaid at nMonthsToEnd - 1
    };

    struct LoanResult {
        Id const* pId; ///< same as ParamsController::Id
        PaymentBreakdown paymentDue;
    };

    virtual ~LoanController() = default;

    /** @param[in]  nMonthsDone  The absolute nMonth from the start of simulation.
     *  @param[in]  amountSpare  [optional] The available equity to consider in the aggressive controller
     */
    virtual LoanResult processMonth(tUint nMonthsDone, tAmount amountSpare) = 0;

    virtual bool isPaidOff() const = 0;

    virtual void makeCheckPoint() = 0;
    virtual void resetToCheckPoint() = 0;
};

class LoanControllerNor final : public LoanController {
  public:
    LoanControllerNor(std::unique_ptr<ParamsController>&& pParamsController, std::unique_ptr<ParamsHandler>&& pParamsHandler);

    LoanResult processMonth(tUint nMonthsDone, tAmount amountSpare) override;

    inline bool isPaidOff() const override { return _loanHandler.amountNow() == 0.0; }

    void makeCheckPoint() override { _loanHandler.makeCheckPoint(); };
    void resetToCheckPoint() override { _loanHandler.resetToCheckPoint(); };

  private:
    std::unique_ptr<ParamsController> _pParams;
    LoanHandlerNor _loanHandler;
};

class LoanControllerAgg final : public LoanController {
  public:
    LoanControllerAgg(
        std::unique_ptr<ParamsController>&& pParamsController,
        std::unique_ptr<ParamsHandler>&& pParamsHandler,
        std::unique_ptr<AcceleratedPay::Params>&& pParamsAccPay,
        std::unique_ptr<LoanPayoff::Params>&& pParamsPayoff,
        std::vector<Bool>&& monthsAllowed ///< the months where aggressive options can be used
    );

    // uses the amountSpare to consider early payoff or an extra principle payment.
    LoanResult processMonth(tUint nMonthsDone, tAmount amountSpare) override;

    inline bool isPaidOff() const override { return _loanHandler.amountNow() == 0.0; }

    void makeCheckPoint() override { _loanHandler.makeCheckPoint(); };
    void resetToCheckPoint() override { _loanHandler.resetToCheckPoint(); };

  private:
    std::unique_ptr<ParamsController> _pParams;
    LoanHandlerAgg _loanHandler;
    std::vector<Bool> _monthsAllowed;
};

class LoanControllerConsv final : public LoanController {
  public:
    LoanControllerConsv(
        std::unique_ptr<ParamsController>&& pParamsController,
        std::unique_ptr<ParamsHandler>&& pParamsHandler,
        std::unique_ptr<LoanHoliday::Params>&& pParamsHoliday);

    LoanResult processMonth(tUint nMonthsDone, tAmount amountSpare) override;

    inline bool isPaidOff() const override { return _loanHandler.amountNow() == 0.0; }

    void makeCheckPoint() override { _loanHandler.makeCheckPoint(); };
    void resetToCheckPoint() override { _loanHandler.resetToCheckPoint(); };

  private:
    std::unique_ptr<ParamsController> _pParams;
    LoanHandlerConsv _loanHandler;
};

} // namespace loan
} // namespace fisim

#endif /* FISIM_LOAN_CONTROLLER_H_ */

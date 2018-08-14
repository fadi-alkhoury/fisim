#ifndef SRC_LOAN_HANDLER_H_
#define SRC_LOAN_HANDLER_H_

#include "acceleratedpay.h"
#include "loan.h"
#include "loanholiday.h"
#include "loanpayoff.h"
#include "repayment_option.h"
#include "types.h"

#include <memory>

namespace fisim {
namespace loan {

struct LoanSettings {
    std::vector<tFloat> interestRatesAllMonths; //< monthly interest rates until the end of sim
    tRepaymentOption repaymentOption;
    tAmount monthlyPayment = 0.0; //< The monthly payment. Only needed for tRepaymentOption::monthlyPayment
    tAmount monthlyPaymentMax;    //< If the required monthly payment exceeds this value, negative amortization will occur
};
struct ParamsHandler {
    Loan loan;
    LoanSettings settings;
};
struct PaymentBreakdown {
    tAmount principle = 0.0;        ///< Normal contribution towards paying off the loan. Negative amortization can occur here.
    tAmount specialRepayment = 0.0; ///< Used for: issuing the loan (negative), extra repayment in a month, payoff of the loan.
    tAmount interest = 0.0;
};

class LoanHandlerNor final {
  public:
    explicit LoanHandlerNor(std::unique_ptr<ParamsHandler>&& pParamsHandler);

    PaymentBreakdown processMonth(tUint nMonthsDone);
    PaymentBreakdown processFirstMonth(tFloat fractionInterest, tFloat fractionRepayment);

    tAmount processRepayment(); ///< repays the full outstanding balance

    tAmount amountNow() const { return _pParamsHandler->loan.amount; }
    tAmount interestPaid() const { return _pParamsHandler->loan.interestPaid; }

    void makeCheckPoint() { _pParamsHandler->loan.makeCheckPoint(); }
    void resetToCheckPoint() { _pParamsHandler->loan.resetToCheckPoint(); }

  protected:
    std::unique_ptr<ParamsHandler> _pParamsHandler;
};

class LoanHandlerAgg final {
  public:
    LoanHandlerAgg(std::unique_ptr<ParamsHandler>&& pParamsHandler, std::unique_ptr<AcceleratedPay::Params> pParamsAccPay, std::unique_ptr<LoanPayoff::Params> pParamsPayoff);

    PaymentBreakdown processMonth(tUint nMonthsDone, tAmount amountSpare = 0.0);
    PaymentBreakdown processFirstMonth(tFloat fractionInterest, tFloat fractionRepayment);

    Loan::LoanStateChange processPayOffOption(tUint nMonthsDone, tAmount amountSpare, tUint nMonthsLoan);

    tAmount processRepayment();                        ///< see LoanHandlerNor
    tAmount processRepayment(tAmount amountRepayment); ///< repays amountRepayment, up to the remaining balance outstanding balance

    tAmount amountNow() const { return _pParamsHandler->loan.amount; }
    tAmount interestPaid() const { return _pParamsHandler->loan.interestPaid; }

    void makeCheckPoint();
    void resetToCheckPoint();

  private:
    std::unique_ptr<ParamsHandler> _pParamsHandler;
    AcceleratedPay _accPay;
    LoanPayoff _payoff;
};

class LoanHandlerConsv final {
  public:
    LoanHandlerConsv(std::unique_ptr<ParamsHandler>&& pParamsHandler, std::unique_ptr<LoanHoliday::Params>&& pParamsHoliday);

    PaymentBreakdown processMonth(tUint nMonthsDone);
    PaymentBreakdown processFirstMonth(tFloat fractionInterest, tFloat fractionRepayment);

    tAmount processRepayment(); ///< see LoanHandlerNor

    tAmount amountNow() const { return _pParamsHandler->loan.amount; }
    tAmount interestPaid() const { return _pParamsHandler->loan.interestPaid; }

    void makeCheckPoint();
    void resetToCheckPoint();

  private:
    std::unique_ptr<ParamsHandler> _pParamsHandler;
    LoanHoliday _loanHoliday;
};

} // namespace loan
} // namespace fisim

#endif /* SRC_LOAN_HANDLER_H_ */

/*
 * FISIM_test.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: me
 */

#include "loan_controller.h"
#include "math.h"
#include <gtest/gtest.h>

namespace fisim {

TEST(LoanControllerNorTest, test00) {
    // TODO These test are not really needed and can be removed. all that is needed is to test triggering the offset and the lonhandler.

    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    auto loanHandler = loanhandler::LoanHandler(std::move(loan), std::move(params));
    tUint nMonthsLoan = 20;
    tUint nMonthsOffset = 0;
    auto loanController = loancontroller::LoanController_Nor(std::move(loanHandler), nMonthsLoan, nMonthsOffset);

    loan::tPaymentBreakdown result{0.0, 0.0};

    auto interestPaid = 0.0;
    for (auto nMonthsDone = 0; nMonthsDone < 10; nMonthsDone++) {
        result = loanController.processMonth(nMonthsDone, 1000);
        EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0 * pow(0.98, nMonthsDone + 1));
        EXPECT_DOUBLE_EQ(result.interest, loanController.amountNow() / 0.98 * 0.01);
        EXPECT_DOUBLE_EQ(result.principle, loanController.amountNow() / 0.98 * 0.02);
        interestPaid += result.interest;
        EXPECT_DOUBLE_EQ(loanController.interestPaid(), interestPaid);
    }

    loanState.amount = 100000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan1 = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params1{};
    params1.monthlyPaymentCap = 1000.0;
    std::vector<tFloat> factors{0.01, 0.01, 0.01, 0.0, 0.02};
    params1.rateInterest = std::make_unique<rate::RateVariable>(std::move(factors));
    factors = {0.01, 0.01, 0.01, 0.0, 0.01};
    params1.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    auto loanHandler1 = loanhandler::LoanHandler(std::move(loan1), std::move(params1));
    auto loanController1 = loancontroller::LoanController_Nor(std::move(loanHandler1), nMonthsLoan, nMonthsOffset);

    result = loanController1.processMonth(0, 3000);
    EXPECT_DOUBLE_EQ(loanController1.amountNow(), 100000.0);
    EXPECT_DOUBLE_EQ(result.interest, 1000);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    interestPaid = result.interest;
    EXPECT_DOUBLE_EQ(loanController1.interestPaid(), interestPaid);

    result = loanController1.processMonth(4, 3000);
    EXPECT_DOUBLE_EQ(loanController1.amountNow(), 101000.0);
    EXPECT_DOUBLE_EQ(result.interest, 1000);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanController1.interestPaid(), interestPaid);

    loanState.amount = 40000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan2 = loan::Loan(std::move(loanState));
    loanhandler::ParamsLoanHandler params2{};
    params2.monthlyPaymentCap = 1000.0;
    factors = {0.01, 0.01, 0.01, 0.0, 0.02};
    params2.rateInterest = std::make_unique<rate::RateVariable>(std::move(factors));
    factors = {0.01, 0.01, 0.01, 0.0, 0.01};
    params2.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    auto loanHandler2 = loanhandler::LoanHandler(std::move(loan2), std::move(params2));
    auto loanController2 = loancontroller::LoanController_Nor(std::move(loanHandler2), nMonthsLoan, nMonthsOffset);
    result = loanController2.processMonth(4, 2500);
    EXPECT_DOUBLE_EQ(loanController2.amountNow(), 40000.0 - 200.0);
    EXPECT_DOUBLE_EQ(result.interest, 800);
    EXPECT_DOUBLE_EQ(result.principle, 200);
    interestPaid = result.interest;
    EXPECT_DOUBLE_EQ(loanController2.interestPaid(), interestPaid);

    // negative amortization with the adjustable interest rate loan
    loanState.amount = 60000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan3 = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params3{};
    params3.monthlyPaymentCap = 1000.0;
    factors = {0.01, 0.01, 0.01, 0.0, 0.02};
    params3.rateInterest = std::make_unique<rate::RateVariable>(std::move(factors));
    factors = {0.01, 0.01, 0.01, 0.0, 0.01};
    params3.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    auto loanHandler3 = loanhandler::LoanHandler(std::move(loan3), std::move(params3));
    auto loanController3 = loancontroller::LoanController_Nor(std::move(loanHandler3), nMonthsLoan, nMonthsOffset);
    result = loanController3.processMonth(4, 2500);
    EXPECT_DOUBLE_EQ(loanController3.amountNow(), 60000.0 + 200.0);
    EXPECT_DOUBLE_EQ(result.interest, 1000);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    interestPaid = result.interest;
    EXPECT_DOUBLE_EQ(loanController3.interestPaid(), interestPaid);

    // repayment
    loanState.amount = 100.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan4 = loan::Loan(std::move(loanState));
    loanhandler::ParamsLoanHandler params4{};
    params4.monthlyPaymentCap = 1000.0;
    params4.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    factors = {0.01, 0.01, 0.01, 0.0, 1};
    params4.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    auto loanHandler4 = loanhandler::LoanHandler(std::move(loan4), std::move(params4));
    auto loanController4 = loancontroller::LoanController_Nor(std::move(loanHandler4), nMonthsLoan, nMonthsOffset);

    result = loanController4.processMonth(4, 1000);
    EXPECT_DOUBLE_EQ(loanController4.amountNow(), 0.0);
    EXPECT_DOUBLE_EQ(result.interest, 1);
    EXPECT_DOUBLE_EQ(result.principle, 100);
    EXPECT_DOUBLE_EQ(loanController4.interestPaid(), result.interest);
}

TEST(LoanControllerNorTest, offset) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    auto loanHandler = loanhandler::LoanHandler(std::move(loan), std::move(params));
    tUint nMonthsLoan = 20;
    tUint nMonthsOffset = 1;
    auto loanController = loancontroller::LoanController_Nor(std::move(loanHandler), nMonthsLoan, nMonthsOffset);

    loan::tPaymentBreakdown result = {0.0, 0.0};

    // offset -- do nothing
    result = loanController.processMonth(0, 1000);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(result.interest, 0);
    EXPECT_DOUBLE_EQ(result.principle, 0);
}

} // namespace fisim

/*
 * acceleratedpay_test.cc
 *
 *  Created on: Apr 15, 2019
 *      Author: me
 */

#include "acceleratedpay.h"

#include "math.h"
#include <gtest/gtest.h>

namespace fisim {

TEST(AcceleratedPayfTest, amountUnschedPossible) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay = loanhandler::AcceleratedPay(std::move(paramsAcc));

    tAmount amountExtra = 0;
    amountExtra = accPay.amountUnschedPossible(10000, 0); // month disabled
    EXPECT_DOUBLE_EQ(amountExtra, 0);
    amountExtra = accPay.amountUnschedPossible(10000, 2); // amount available exceeds remaining allowance
    EXPECT_DOUBLE_EQ(amountExtra, 100);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 0;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay1 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    amountExtra = accPay1.amountUnschedPossible(50, 3); // month enabled but allowance used up
    EXPECT_DOUBLE_EQ(amountExtra, 0);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 10;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay2 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    amountExtra = accPay2.amountUnschedPossible(50, 3); // month enabled but allowance below min
    EXPECT_DOUBLE_EQ(amountExtra, 0);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay3 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    amountExtra = accPay3.amountUnschedPossible(50, 3); // month enabled, amount available is the min
    EXPECT_DOUBLE_EQ(amountExtra, 50);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay4 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    amountExtra = accPay4.amountUnschedPossible(75, 3); // month enabled, min < amount available < remaining allowance
    EXPECT_DOUBLE_EQ(amountExtra, 75);
}

TEST(AcceleratedPayfTest, processUnschedPayment) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay = loanhandler::AcceleratedPay(std::move(paramsAcc));

    auto amountPaid = accPay.processUnschedPayment(loan, 10);
    EXPECT_DOUBLE_EQ(amountPaid, 10);
    EXPECT_DOUBLE_EQ(loan.amountNow(), 990);
    EXPECT_DOUBLE_EQ(accPay.thisYearRemaining(), 90);

    amountPaid = accPay.processUnschedPayment(loan, 90);
    EXPECT_DOUBLE_EQ(amountPaid, 90);
    EXPECT_DOUBLE_EQ(loan.amountNow(), 900);
    EXPECT_DOUBLE_EQ(accPay.thisYearRemaining(), 0);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay1 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    EXPECT_DEATH({ amountPaid = accPay1.processUnschedPayment(loan, 1000); }, "");
}

TEST(AcceleratedPayfTest, isAnnualReset) {
    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay = loanhandler::AcceleratedPay(std::move(paramsAcc));

    for (auto i = 0u; i < 12; ++i) {
        EXPECT_EQ(accPay.isAnnualReset(i), false);
    }
    for (auto i = 1u; i < 10; ++i) {
        EXPECT_EQ(accPay.isAnnualReset(12 * i), true);
    }
    for (auto i = 13u; i < 24; ++i) {
        EXPECT_EQ(accPay.isAnnualReset(i), false);
    }

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 3;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay1 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    for (auto i = 0u; i < 9; ++i) {
        EXPECT_EQ(accPay1.isAnnualReset(i), false);
    }
    for (auto i = 1u; i < 10; ++i) {
        EXPECT_EQ(accPay1.isAnnualReset(12 * i - 3), true);
    }
    for (auto i = 10u; i < 21; ++i) {
        EXPECT_EQ(accPay1.isAnnualReset(i), false);
    }

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 3;
    paramsAcc.isAnnualReset = false;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay2 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    for (auto i = 0u; i < 1200; ++i) {
        EXPECT_EQ(accPay2.isAnnualReset(i), false);
    }
}

TEST(AcceleratedPayfTest, annualReset) {
    loanhandler::ParamsAccelerate paramsAcc;
    tAmount amountNowLoan = 1500;
    tAmount amountInitLoan = 2000;

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;
    auto accPay = loanhandler::AcceleratedPay(std::move(paramsAcc));
    accPay.annualReset(amountNowLoan, amountInitLoan);
    EXPECT_DOUBLE_EQ(accPay.thisYearRemaining(), 200);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 200;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = false;
    auto accPay1 = loanhandler::AcceleratedPay(std::move(paramsAcc));
    accPay1.annualReset(amountNowLoan, amountInitLoan);
    EXPECT_DOUBLE_EQ(accPay1.thisYearRemaining(), 150);
}

} // namespace fisim

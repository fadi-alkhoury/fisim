#include "loan.h"
#include <gtest/gtest.h>

namespace fisim {
namespace FISIM_test {

using Loan = fisim::loan::Loan;
using LoanStateChange = fisim::loan::Loan::LoanStateChange;

TEST(LoanTest, test00) {
    Loan loan{};
    loan.amount = 1000.0;
    loan.amountInit = 1000.0;
    loan.interestPaid = 0;

    LoanStateChange stateChange;

    for (auto i = 1; i < 100; ++i) {
        stateChange.interest = 1;
        stateChange.principle = 10;
        loan.processStateChange(stateChange);

        EXPECT_DOUBLE_EQ(stateChange.interest, 1.0);
        EXPECT_DOUBLE_EQ(stateChange.principle, 10.0);
        EXPECT_DOUBLE_EQ(loan.amountInit, 1000.0);
        EXPECT_DOUBLE_EQ(loan.amount, 1000.0 - 10 * i);
        EXPECT_DOUBLE_EQ(loan.interestPaid, i);
    }
    loan.processStateChange(stateChange);

    EXPECT_DOUBLE_EQ(stateChange.interest, 1.0);
    EXPECT_DOUBLE_EQ(stateChange.principle, 10.0);
    EXPECT_DOUBLE_EQ(loan.amountInit, 1000.0);
    EXPECT_DOUBLE_EQ(loan.amount, 0.0);
    EXPECT_DOUBLE_EQ(loan.interestPaid, 100);
}

// negative principle
TEST(LoanTest, test01) {
    Loan loan{};
    loan.amount = 1000.0;
    loan.amountInit = 1000.0;
    loan.interestPaid = 100.0;

    LoanStateChange stateChange;

    for (auto i = 1; i < 100; ++i) {
        stateChange.interest = 1;
        stateChange.principle = 10;
        loan.processStateChange(stateChange);

        EXPECT_DOUBLE_EQ(stateChange.interest, 1.0);
        EXPECT_DOUBLE_EQ(stateChange.principle, 10.0);
        EXPECT_DOUBLE_EQ(loan.amountInit, 1000.0);
        EXPECT_DOUBLE_EQ(loan.amount, 1000.0 - 10 * i);
        EXPECT_DOUBLE_EQ(loan.interestPaid, i + 100);
    }

    stateChange.principle = -100;
    loan.processStateChange(stateChange);

    EXPECT_DOUBLE_EQ(stateChange.interest, 1.0);
    EXPECT_DOUBLE_EQ(stateChange.principle, -100.0);
    EXPECT_DOUBLE_EQ(loan.amountInit, 1000.0);
    EXPECT_DOUBLE_EQ(loan.amount, 110);
    EXPECT_DOUBLE_EQ(loan.interestPaid, 100 + 100);

    stateChange.interest = 10.2;
    stateChange.principle = 1000;
    loan.processStateChange(stateChange);

    EXPECT_DOUBLE_EQ(stateChange.interest, 10.2);
    EXPECT_DOUBLE_EQ(stateChange.principle, 110.0);
    EXPECT_DOUBLE_EQ(loan.amountInit, 1000.0);
    EXPECT_DOUBLE_EQ(loan.amount, 0);
    EXPECT_DOUBLE_EQ(loan.interestPaid, 110.2 + 100);
}

} // namespace FISIM_test
} // namespace fisim

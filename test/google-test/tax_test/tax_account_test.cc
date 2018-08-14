/*
 * tax_account_test.cc
 *
 *  Created on: Apr 9, 2019
 *      Author: me
 */

#include "tax_account.h"

#include <cmath>
#include <gtest/gtest.h>

///
/// \note These tests neeeds to be updated for the new interfaces. They are excluded in cmake for now.
///

namespace fisim {
namespace tax {

// gtest recommends naming the test suite (not test) *DeathTest so it can avoid threading issues
TEST(DeathTest, TaxAccountDeath) {

    tInt idTax1 = 1;
    std::vector<tAmount> incomeAmounts1{1, 10};
    std::vector<tFloat> taxRates1{0.0, 0.0}; // 0 tax
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::move(incomeAmounts1);
    progTax1.taxRates = std::move(taxRates1);

    // if starting at a new tax year, must die if the tax return and taxrate are not NaNs (placeholders)
    EXPECT_DEATH(
        {
            CalendarMonth monthInit1 = CalendarMonth::feb;
            CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
            CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
            tax::TaxAccountConfig config1;
            config1.idTax = idTax1;
            config1.monthInit = monthInit1;
            config1.taxYearStartMonth = taxYearStartMonth1;
            config1.taxReturnMonth = taxReturnMonth1;
            config1.progTaxDataPoints = std::move(progTax1);
            config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

            tAmount amountTaxReturn = 29;
            tFloat taxRate = 0.03;
            auto state1 = TaxAccountState(1, 1, 1, taxRate, amountTaxReturn);
            tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));
        },
        "");

    EXPECT_DEATH(
        {
            CalendarMonth monthInit1 = CalendarMonth::feb;
            CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
            CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
            tax::TaxAccountConfig config1;
            config1.idTax = idTax1;
            config1.monthInit = monthInit1;
            config1.taxYearStartMonth = taxYearStartMonth1;
            config1.taxReturnMonth = taxReturnMonth1;
            config1.progTaxDataPoints = std::move(progTax1);
            config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

            tAmount amountTaxReturn = std::numeric_limits<tAmount>::quiet_NaN();
            tFloat taxRate = 0.03;
            auto state1 = TaxAccountState(1, 1, 1, taxRate, amountTaxReturn);
            tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));
        },
        "");

    EXPECT_DEATH(
        {
            CalendarMonth monthInit1 = CalendarMonth::feb;
            CalendarMonth taxYearStarCalendarMonth1 = CalendarMonth::feb;
            CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
            tax::TaxAccountConfig config1;
            config1.idTax = idTax1;
            config1.monthInit = monthInit1;
            config1.taxYearStartMonth = taxYearStartMonth1;
            config1.taxReturnMonth = taxReturnMonth1;
            config1.progTaxDataPoints = std::move(progTax1);
            config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

            tAmount amountTaxReturn = 3;
            tFloat taxRate = std::numeric_limits<tAmount>::quiet_NaN();
            auto state1 = TaxAccountState(1, 1, 1, taxRate, amountTaxReturn);
            tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));
        },
        "");

    // if starting past the tax return month, must die if the tax return and taxrate are not NaNs (placeholders)
    EXPECT_DEATH(
        {
            CalendarMonth monthInit1 = CalendarMonth::aug;
            CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
            CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
            tax::TaxAccountConfig config1;
            config1.idTax = idTax1;
            config1.monthInit = monthInit1;
            config1.taxYearStartMonth = taxYearStartMonth1;
            config1.taxReturnMonth = taxReturnMonth1;
            config1.progTaxDataPoints = std::move(progTax1);
            config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

            tAmount amountTaxReturn = 29;
            tFloat taxRate = 0.03;
            auto state1 = TaxAccountState(1, 1, 1, taxRate, amountTaxReturn);
            tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));
        },
        "");

    EXPECT_DEATH(
        {
            CalendarMonth monthInit1 = CalendarMonth::sep;
            CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
            CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
            tax::TaxAccountConfig config1;
            config1.idTax = idTax1;
            config1.monthInit = monthInit1;
            config1.taxYearStartMonth = taxYearStartMonth1;
            config1.taxReturnMonth = taxReturnMonth1;
            config1.progTaxDataPoints = std::move(progTax1);
            config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

            tAmount amountTaxReturn = std::numeric_limits<tAmount>::quiet_NaN();
            tFloat taxRate = 0.03;
            auto state1 = TaxAccountState(1, 1, 1, taxRate, amountTaxReturn);
            tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));
        },
        "");

    EXPECT_DEATH(
        {
            CalendarMonth monthInit1 = CalendarMonth::jul;
            CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
            CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
            tax::TaxAccountConfig config1;
            config1.idTax = idTax1;
            config1.monthInit = monthInit1;
            config1.taxYearStartMonth = taxYearStartMonth1;
            config1.taxReturnMonth = taxReturnMonth1;
            config1.progTaxDataPoints = std::move(progTax1);
            config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

            tAmount amountTaxReturn = 3;
            tFloat taxRate = std::numeric_limits<tAmount>::quiet_NaN();
            auto state1 = TaxAccountState(1, 1, 1, taxRate, amountTaxReturn);
            tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));
        },
        "");
}

// starting at a new tax year -> must compute the tax return and new taxrate
TEST(TaxAccount, testNewYear) {
    tInt idTax1 = 1;
    std::vector<tAmount> incomeAmounts1{1, 1000};
    std::vector<tFloat> taxRates1{0.05, 0.50};
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::move(incomeAmounts1);
    progTax1.taxRates = std::move(taxRates1);
    CalendarMonth monthInit1 = CalendarMonth::feb;
    CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
    CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
    tax::TaxAccountConfig config1;
    config1.idTax = idTax1;
    config1.monthInit = monthInit1;
    config1.taxYearStartMonth = taxYearStartMonth1;
    config1.taxReturnMonth = taxReturnMonth1;
    config1.progTaxDataPoints = std::move(progTax1);
    config1.amountDeductible = GrowableAmount(10, 1.5, 12u, 3u);

    tAmount earnings = 100;
    tAmount deductions = 20;
    tAmount taxPaid = 20;
    auto state1 = TaxAccountState(earnings, deductions, taxPaid);
    tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));

    EXPECT_DOUBLE_EQ(taxAccount1.state.taxRate, 0.05 + (100 - 20 - 10 - 1) * 0.45 / 999.0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.amountTaxReturn, 20 - (100 - 20 - 10) * taxAccount1.state.taxRate);
    EXPECT_EQ(taxAccount1.state.nMonthTaxReturn, 4);
    EXPECT_DOUBLE_EQ(taxAccount1.state.earnings, 0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.taxPaid, 0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.deductions, 0);
}

// starting before the TR -> nothing special
TEST(TaxAccount, testBeforeTr) {
    tInt idTax1 = 1;
    std::vector<tAmount> incomeAmounts1{1, 1000};
    std::vector<tFloat> taxRates1{0.05, 0.50};
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::move(incomeAmounts1);
    progTax1.taxRates = std::move(taxRates1);
    CalendarMonth monthInit1 = CalendarMonth::sep;
    CalendarMonth taxYearStartMonth1 = CalendarMonth::jun;
    CalendarMonth taxReturnMonth1 = CalendarMonth::nov;
    tax::TaxAccountConfig config1;
    config1.idTax = idTax1;
    config1.monthInit = monthInit1;
    config1.taxYearStartMonth = taxYearStartMonth1;
    config1.taxReturnMonth = taxReturnMonth1;
    config1.progTaxDataPoints = std::move(progTax1);
    config1.amountDeductible = GrowableAmount(10, 1.5, 12u, 3u);

    tFloat taxRate = 0.1;
    tAmount amountTaxReturn = 52;
    tAmount earnings = 100;
    tAmount deductions = 20;
    tAmount taxPaid = 40;
    auto state1 = TaxAccountState(earnings, deductions, taxPaid, amountTaxReturn, taxRate);
    tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));

    EXPECT_DOUBLE_EQ(taxAccount1.state.taxRate, 0.1);
    EXPECT_DOUBLE_EQ(taxAccount1.state.amountTaxReturn, 52);
    EXPECT_EQ(taxAccount1.state.nMonthTaxReturn, 2);
    EXPECT_DOUBLE_EQ(taxAccount1.state.earnings, 100);
    EXPECT_DOUBLE_EQ(taxAccount1.state.taxPaid, 40);
    EXPECT_DOUBLE_EQ(taxAccount1.state.deductions, 20);
}

// starting at the TR -> nothing special
TEST(TaxAccount, testAtTr) {
    tInt idTax1 = 1;
    std::vector<tAmount> incomeAmounts1{1, 1000};
    std::vector<tFloat> taxRates1{0.05, 0.50};
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::move(incomeAmounts1);
    progTax1.taxRates = std::move(taxRates1);
    CalendarMonth monthInit1 = CalendarMonth::nov;
    CalendarMonth taxYearStartMonth1 = CalendarMonth::jun;
    CalendarMonth taxReturnMonth1 = CalendarMonth::nov;
    tax::TaxAccountConfig config1;
    config1.idTax = idTax1;
    config1.monthInit = monthInit1;
    config1.taxYearStartMonth = taxYearStartMonth1;
    config1.taxReturnMonth = taxReturnMonth1;
    config1.progTaxDataPoints = std::move(progTax1);
    config1.amountDeductible = GrowableAmount(10, 1.5, 12u, 3u);

    tFloat taxRate = 0.1;
    tAmount amountTaxReturn = 52;
    tAmount earnings = 100;
    tAmount deductions = 20;
    tAmount taxPaid = 40;
    auto state1 = TaxAccountState(earnings, deductions, taxPaid, amountTaxReturn, taxRate);
    tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));

    EXPECT_DOUBLE_EQ(taxAccount1.state.taxRate, 0.1);
    EXPECT_DOUBLE_EQ(taxAccount1.state.amountTaxReturn, 52);
    EXPECT_EQ(taxAccount1.state.nMonthTaxReturn, 0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.earnings, 100);
    EXPECT_DOUBLE_EQ(taxAccount1.state.taxPaid, 40);
    EXPECT_DOUBLE_EQ(taxAccount1.state.deductions, 20);
}

// starting past the TR -> nothing special
TEST(TaxAccount, testPastTr) {
    tInt idTax1 = 1;
    std::vector<tAmount> incomeAmounts1{1, 1000};
    std::vector<tFloat> taxRates1{0.05, 0.50};
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::move(incomeAmounts1);
    progTax1.taxRates = std::move(taxRates1);
    CalendarMonth monthInit1 = CalendarMonth::feb;
    CalendarMonth taxYearStartMonth1 = CalendarMonth::jun;
    CalendarMonth taxReturnMonth1 = CalendarMonth::nov;
    tax::TaxAccountConfig config1;
    config1.idTax = idTax1;
    config1.monthInit = monthInit1;
    config1.taxYearStartMonth = taxYearStartMonth1;
    config1.taxReturnMonth = taxReturnMonth1;
    config1.progTaxDataPoints = std::move(progTax1);
    config1.amountDeductible = GrowableAmount(10, 1.5, 12u, 3u);

    tAmount earnings = 100;
    tAmount deductions = 20;
    tAmount taxPaid = 40;
    auto state1 = TaxAccountState(earnings, deductions, taxPaid);
    tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));

    EXPECT_EQ(std::isnan(state1.taxRate), true);
    EXPECT_EQ(std::isnan(state1.amountTaxReturn), true);
    EXPECT_EQ(taxAccount1.state.nMonthTaxReturn, 9);
    EXPECT_DOUBLE_EQ(taxAccount1.state.earnings, 100);
    EXPECT_DOUBLE_EQ(taxAccount1.state.taxPaid, 40);
    EXPECT_DOUBLE_EQ(taxAccount1.state.deductions, 20);
}

TEST(TaxAccount, testProcessTaxYear) {
    tInt idTax1 = 1;
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::vector<tAmount>{1, 1000};
    progTax1.taxRates = std::vector<tFloat>{0.05, 0.50};
    CalendarMonth monthInit1 = CalendarMonth::jan;
    CalendarMonth taxYearStartMonth1 = CalendarMonth::feb;
    CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
    tax::TaxAccountConfig config1;
    config1.idTax = idTax1;
    config1.monthInit = monthInit1;
    config1.taxYearStartMonth = taxYearStartMonth1;
    config1.taxReturnMonth = taxReturnMonth1;
    config1.progTaxDataPoints = std::move(progTax1);
    config1.amountDeductible = GrowableAmount(10, 1.5, 12u, 3u);

    tAmount earnings = 100;
    tAmount deductions = 20;
    tAmount taxPaid = 20;
    auto state1 = TaxAccountState(earnings, deductions, taxPaid);
    tax::TaxAccount taxAccount1(std::move(state1), std::move(config1));

    taxAccount1.processTaxYear(1);

    EXPECT_DOUBLE_EQ(taxAccount1.state.taxRate, 0.05 + (100 - 20 - 10 - 1) * 0.45 / 999.0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.amountTaxReturn, 20 - (100 - 20 - 10) * taxAccount1.state.taxRate);
    EXPECT_EQ(taxAccount1.state.nMonthTaxReturn, 5);
    EXPECT_DOUBLE_EQ(taxAccount1.state.earnings, 0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.taxPaid, 0);
    EXPECT_DOUBLE_EQ(taxAccount1.state.deductions, 0);
}

} // namespace tax
} // namespace fisim

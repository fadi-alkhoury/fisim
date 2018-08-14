/*
 * tax_handler_test.cc
 *
 *  Created on: Apr 9, 2019
 *      Author: me
 */

#include "tax_handler.h"
#include <gtest/gtest.h>

///
/// \note These tests neeeds to be updated for the new interfaces. They are excluded in cmake for now.
///

namespace fisim {
namespace tax {

TEST(TaxHandler, test00) {
    std::vector<tax::TaxAccount> itemsTaxAccount;

    // create two tax accounts for the handler
    tInt idTax1 = 1;
    std::vector<tAmount> incomeAmounts1{1, 10};
    std::vector<tFloat> taxRates1{0.0, 0.0}; // 0 tax
    tax::ProgTaxSpec progTax1;
    progTax1.incomeAmounts = std::move(incomeAmounts1);
    progTax1.taxRates = std::move(taxRates1);
    CalendarMonth monthInit1 = CalendarMonth::feb;
    CalendarMonth taxYearStartMonth1 = CalendarMonth::jan;
    CalendarMonth taxReturnMonth1 = CalendarMonth::jun;
    tax::TaxAccountConfig config1;
    config1.idTax = idTax1;
    config1.monthInit = monthInit1;
    config1.taxYearStartMonth = taxYearStartMonth1;
    config1.taxReturnMonth = taxReturnMonth1;
    config1.progTaxDataPoints = std::move(progTax1);
    config1.amountDeductible = GrowableAmount(100, 1.5, 12u, 3u);

    tFloat taxRate = 0.f;
    tAmount amountTaxReturn = 59.0;
    tAmount earnings = 100.0;
    tAmount deductions = 10.0;
    tAmount taxPaid = 1.0;
    auto state1 = TaxAccountState(earnings, deductions, taxPaid, amountTaxReturn, taxRate);

    tax::TaxAccount taxAccount1{std::move(state1), std::move(config1)};
    itemsTaxAccount.push_back(std::move(taxAccount1));

    tInt idTax2 = 2;
    std::vector<tAmount> incomeAmounts2{1, 10};
    std::vector<tFloat> taxRates2{0.2, 0.2};
    tax::ProgTaxSpec progTax2;
    progTax2.incomeAmounts = std::move(incomeAmounts2);
    progTax2.taxRates = std::move(taxRates2);
    CalendarMonth monthInit2 = CalendarMonth::mar;
    CalendarMonth taxYearStartMonth2 = CalendarMonth::feb;
    CalendarMonth taxReturnMonth2 = CalendarMonth::jan;
    tax::TaxAccountConfig config2;
    config2.idTax = idTax2;
    config2.monthInit = monthInit2;
    config2.taxYearStartMonth = taxYearStartMonth2;
    config2.taxReturnMonth = taxReturnMonth2;
    config2.progTaxDataPoints = std::move(progTax2);
    config2.amountDeductible = GrowableAmount(100, 1.0, 12u, 2u);

    taxRate = 0.10;
    amountTaxReturn = 70;
    auto state2 = TaxAccountState(earnings, deductions, taxPaid, amountTaxReturn, taxRate);

    itemsTaxAccount.emplace_back(std::move(state2), std::move(config2));
    tax::TaxHandler taxHandler{std::move(itemsTaxAccount)};
    //
    //	{
    //		tInt idTax = 1;
    //		tAmount amount = 1000;
    //		tFloat rate = 0;
    //		bool isAppliedAtSource = true;
    //		tAmount taxDue = taxHandler.processTax(idTax, amount, rate, isAppliedAtSource);
    //		EXPECT_DOUBLE_EQ(taxDue, 0);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 1);
    //
    //		amount = -1000;
    //		taxDue = taxHandler.processTax(idTax, amount, rate, isAppliedAtSource);
    //		EXPECT_DOUBLE_EQ(taxDue, 0);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100); // outgoing payments don't affect earnings
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10); // no changes because of 0% tax deduction
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 1);
    //
    //		idTax = 2;
    //		rate = 0.10;
    //		isAppliedAtSource = false;
    //		taxDue = taxHandler.processTax(idTax, 1000, rate, isAppliedAtSource);
    //		EXPECT_DOUBLE_EQ(taxDue, 0);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 1100); //increased
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 1);
    //
    //		taxDue = taxHandler.processTax(idTax, -100, 1, isAppliedAtSource); //full amount is tax deductible
    //		EXPECT_DOUBLE_EQ(taxDue, 0.0); //not deduction at source
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 1000); //decreased
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 1);
    //
    //		taxDue = taxHandler.processTax(idTax, -990, 1, isAppliedAtSource); //full amount is tax deductible
    //		EXPECT_DOUBLE_EQ(taxDue, 0);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 10); //decrease
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 1); //no change
    //
    //		isAppliedAtSource = true;
    //		taxDue = taxHandler.processTax(idTax, -2, 1, isAppliedAtSource); //full amount is tax deductible
    //		EXPECT_DOUBLE_EQ(taxDue, -2*taxHandler.taxRate(1));
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 8);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 0.8);
    //
    //	}
    //
    //	{	//account 2 end of tax year
    //		auto monthNow = CalendarMonth::jan;
    //		auto nMonthNow = 11u;
    //
    //		taxHandler.processTaxMonth(monthNow ,nMonthNow);
    //
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 10);
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //		EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 0); //reset
    //		EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 0); //reset
    //		EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 0); //reset
    //
    //		// checking itemsUpdateInfo
    //		EXPECT_EQ(*(taxHandler.itemsUpdateInfo[0].pIdTax), 1); //no changes for account 1
    //		EXPECT_DOUBLE_EQ(*(taxHandler.itemsUpdateInfo[0].pAmountTaxReturn), 59);
    //		EXPECT_DOUBLE_EQ(*(taxHandler.itemsUpdateInfo[0].pNmonthUpdate), 4);
    //		EXPECT_DOUBLE_EQ(*(taxHandler.itemsUpdateInfo[0].pTaxRateUpdate), 0);
    //		EXPECT_EQ(*(taxHandler.itemsUpdateInfo[1].pIdTax), 2);
    //		EXPECT_DOUBLE_EQ(*(taxHandler.itemsUpdateInfo[1].pAmountTaxReturn), 0.8); // update: deductions exceeded earnings so all tax paid is returned
    //		EXPECT_DOUBLE_EQ(*(taxHandler.itemsUpdateInfo[1].pNmonthUpdate), 10+12);
    //		EXPECT_DOUBLE_EQ(*(taxHandler.itemsUpdateInfo[1].pTaxRateUpdate), 0.20); //update
    //	}
    //
    //	// checking addTaxDeduction()
    //	taxHandler.addTaxDeduction(1, 60);
    //	EXPECT_DOUBLE_EQ(taxHandler.earnings(0), 1100);
    //	EXPECT_DOUBLE_EQ(taxHandler.deductions(0), 70);
    //	EXPECT_DOUBLE_EQ(taxHandler.taxPaid(0), 1);
    //	EXPECT_DOUBLE_EQ(taxHandler.earnings(1), 0);
    //	EXPECT_DOUBLE_EQ(taxHandler.deductions(1), 0);
    //	EXPECT_DOUBLE_EQ(taxHandler.taxPaid(1), 0);
}

} // namespace tax
} // namespace fisim

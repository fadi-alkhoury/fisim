#include "json_helpers.h"
#include "utils_parsing.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <optional>

namespace fisim {
namespace json {

using loan::LoanControllerAgg;
using loan::LoanControllerConsv;
using loan::LoanControllerNor;
using loan::LoanHandlerAgg;
using loan::LoanHandlerConsv;
using loan::LoanHandlerNor;
using loan::tRepaymentOption;
using tax::TaxAccount;
using Config = tax::TaxAccount::Config;

RelationsLookup parseRelationsRecords(JsonDoc const& document) {
    RelationsLookup relations;
    const auto& doc_relationsRecords = document["relationsRecords"].GetArray();

    std::for_each(doc_relationsRecords.Begin(), doc_relationsRecords.End(), [&relations](const auto& doc_relation) {
        std::optional<tInt> cashflowId;
        std::optional<tInt> loanId;
        std::optional<tInt> taxAccountId;

        for (const auto& member : doc_relation.GetObject()) {
            if (strcmp(member.name.GetString(), "cashflowId") == 0) {
                cashflowId = member.value.GetInt();
            } else if (strcmp(member.name.GetString(), "loanId") == 0) {
                loanId = member.value.GetInt();
            } else if (strcmp(member.name.GetString(), "taxAccountId") == 0) {
                taxAccountId = member.value.GetInt();
            }
        }

        if (cashflowId && taxAccountId) {
            CashflowTaxLink link;
            link.taxAccountId = *taxAccountId;
            link.shouldWithhold = false;
            link.currentRate = 0.f;

            const auto& doc_relationsData = doc_relation["relationData"];

            auto memberItr = doc_relationsData.FindMember("shouldWithhold");
            if (memberItr != doc_relationsData.MemberEnd()) {
                link.shouldWithhold = memberItr->value.GetBool();
            }

            memberItr = doc_relationsData.FindMember("currentRate");
            if (memberItr != doc_relationsData.MemberEnd()) {
                link.currentRate = memberItr->value.GetFloat();
            }

            relations.cashflowTaxLinks[*cashflowId] = link;
        } else if (loanId && taxAccountId) {
            relations.loanTaxLinks[*loanId] = *taxAccountId;
        }
    });

    return relations;
}

LinkToTaxAccount::LinkToTaxAccount(const TaxHandler& taxHandler) : m_pTaxHandler(&taxHandler) {}

void LinkToTaxAccount::operator()(tInt taxId, Cashflow::TaxInfo::NextUpdate& o_taxUpdate) const {
    const auto& account = m_pTaxHandler->account(taxId);

    o_taxUpdate.pNmonth = &account.state.nMonthTaxReturn;
    o_taxUpdate.pRate = &account.state.taxRate;
}

std::vector<Cashflow> parseCashflows(JsonDoc const& document, const CashflowTaxLinkLookupMap& taxLinkLookupMap, LinkToTaxAccount linkToTaxAccount) {
    std::vector<Cashflow> cashflows;

    auto const& doc_cashflows = document["cashflows"].GetArray();
    auto const nCashflows = doc_cashflows.Size();

    cashflows.reserve(nCashflows);
    for (auto i = 0u; i < nCashflows; ++i) {
        auto const& doc_cashflow = doc_cashflows[i];
        auto pParams = std::make_unique<Cashflow::Params>();
        auto& params = *pParams;

        params.id.idCashflow = doc_cashflow["id"].GetInt();
        params.id.idLoan = 0;

        bool isIncome = false;

        const auto amountItr = doc_cashflow.FindMember("amount");
        if (amountItr != doc_cashflow.MemberEnd()) {
            const auto amount = amountItr->value.GetDouble();
            isIncome = amount > 0.0;
            params.uncertainAmount = UncertainAmount(amount);
            params.lossProbability = 0.F;
        } else {
            auto pParamsAmount = std::make_unique<UncertainAmount::Params>();
            auto& paramsAmount = *pParamsAmount;

            const auto amountMin = doc_cashflow["amountMin"].GetDouble();
            isIncome = amountMin > 0.0;

            paramsAmount.amountMin = amountMin;
            paramsAmount.amountMax = doc_cashflow["amountMax"].GetDouble();
            paramsAmount.changeFactorMin = doc_cashflow["changeFactorMin"].GetFloat();
            paramsAmount.changeFactorMax = doc_cashflow["changeFactorMax"].GetFloat();
            paramsAmount.changeInterval = doc_cashflow["changeInterval"].GetUint();
            paramsAmount.intervalElapsedInit = doc_cashflow["intervalElapsed"].GetInt();

            params.uncertainAmount = UncertainAmount(std::move(pParamsAmount));
            params.lossProbability = doc_cashflow["lossProbability"].GetFloat();
        }

        const auto& doc_stepsActive = doc_cashflow["stepsActive"];
        const auto arrSize = doc_stepsActive.Size();
        params.stepsActive.reserve(arrSize);
        for (auto i = 0u; i < arrSize; ++i) {
            params.stepsActive.emplace_back(doc_stepsActive[i].GetBool());
        }

        params.id.idTax = 0; // non taxable
        params.taxInfo.shouldApplyAtSource = false;
        params.taxInfo.rate = 0.F;

        const auto taxLinkItr = taxLinkLookupMap.find(params.id.idCashflow);
        if (taxLinkItr != taxLinkLookupMap.end()) {
            params.id.idTax = taxLinkItr->second.taxAccountId;
            params.taxInfo.shouldApplyAtSource = taxLinkItr->second.shouldWithhold;

            if (params.taxInfo.shouldApplyAtSource) {
                params.taxInfo.rate = taxLinkItr->second.currentRate;
            }
        }

        if (isIncome && params.id.idTax != 0) {
            linkToTaxAccount(params.id.idTax, params.taxInfo.nextUpdate);
        } else { // the deduction rates for costs don't update
            params.taxInfo.nextUpdate.pNmonth = nullptr;
            params.taxInfo.nextUpdate.pRate = nullptr;
        }

        cashflows.emplace_back(std::move(pParams));
    }

    return cashflows;
}

LoansParseResult parseLoans(JsonDoc const& document, const LoanTaxLinkLookupMap& taxLinkLookupMap, CalendarMonth monthInit, tUint nMonths) {
    LoansParseResult parseResults;
    auto const& doc_loans = document["loans"].GetArray();
    auto nloans = doc_loans.Size();
    parseResults.loansData.reserve(nloans);
    parseResults.backdoors.resize(nloans);

    for (auto i = 0u; i < nloans; ++i) {
        const auto& doc_loan = doc_loans[i];

        auto pParamsController = std::make_unique<LoanController::ParamsController>();
        auto& paramsController = *pParamsController;
        parseResults.backdoors[i].pParamsController = pParamsController.get();
        parseResults.backdoors[i].pMonthsAggAllowed = nullptr;

        paramsController.id.idCashflow = 0;
        paramsController.id.idTax = 0;
        paramsController.id.idLoan = doc_loan["id"].GetInt();

        const auto taxIdItr = taxLinkLookupMap.find(paramsController.id.idLoan);
        if (taxIdItr != taxLinkLookupMap.end()) {
            paramsController.id.idTax = taxIdItr->second;
        }

        paramsController.nMonthsOffset = doc_loan["nMonthsToStart"].GetInt();
        paramsController.nMonthsToEnd = doc_loan["duration"].GetInt() + paramsController.nMonthsOffset;
        assert(static_cast<tInt>(paramsController.nMonthsToEnd) > paramsController.nMonthsOffset);

        paramsController.fracFirstMonthInterest = doc_loan["firstMonthInterestFraction"].GetFloat();

        paramsController.fracFirstMonthRepayment = 0.0F;
        auto doc_memberItr = doc_loan.FindMember("firstMonthRepaymentFraction");
        if (doc_memberItr != doc_loan.MemberEnd()) {
            paramsController.fracFirstMonthRepayment = doc_memberItr->value.GetFloat();
        }

        paramsController.isForceRepay = false;
        doc_memberItr = doc_loan.FindMember("isForceRepay");
        if (doc_memberItr != doc_loan.MemberEnd()) {
            paramsController.isForceRepay = doc_memberItr->value.GetBool();
        }

        // ParamsHandler
        auto pParamsHandler = std::make_unique<fisim::loan::ParamsHandler>();
        auto& paramsHandler = *pParamsHandler;

        paramsHandler.loan.amount = doc_loan["amount"].GetDouble();
        paramsHandler.loan.amountInit = paramsHandler.loan.amount;
        paramsHandler.loan.interestPaid = 0.0;

        // get the chart points and interpolate to get the interest for all months
        auto& doc_interestMonthsArray = doc_loan["interestMonthsArray"];
        auto& doc_interestRatesArray = doc_loan["interestRatesArray"];
        auto arrSize = doc_interestMonthsArray.Size();

        std::vector<tFloat> interestRatesArray;
        std::vector<tUint> interestMonthsArray;
        interestRatesArray.reserve(arrSize);
        interestMonthsArray.reserve(arrSize);
        for (auto i = 0u; i < arrSize; ++i) {
            interestMonthsArray.push_back(doc_interestMonthsArray[i].GetUint());
            interestRatesArray.push_back(doc_interestRatesArray[i].GetFloat() / 12.0f);
        }

        auto& settings = paramsHandler.settings;
        settings.interestRatesAllMonths = interpolateY(interestRatesArray, interestMonthsArray, nMonths);

        // Note: the front-end fix amortization option is also represented in tRepaymentOption::monthlypayment
        settings.repaymentOption = static_cast<tRepaymentOption>(doc_loan["repaymentOption"].GetUint());
        settings.monthlyPayment = 0.0; // interest-only unless input exists in json data
        doc_memberItr = doc_loan.FindMember("monthlyPayment");
        if (doc_memberItr != doc_loan.MemberEnd()) {
            settings.monthlyPayment = doc_memberItr->value.GetDouble();
        }

        settings.monthlyPaymentMax = std::numeric_limits<tAmount>::max();
        doc_memberItr = doc_loan.FindMember("monthlyPaymentMax");
        if (doc_memberItr != doc_loan.MemberEnd()) {
            settings.monthlyPaymentMax = doc_memberItr->value.GetDouble();
        }

        const auto repaymentStrategy = doc_loan["repaymentStrategy"].GetUint();

        if (repaymentStrategy == 1u) {
            std::vector<Bool> monthsStrategyAllowed;
            monthsStrategyAllowed.resize(nMonths, true);

            // ParamsAccPay
            const auto& doc_extraPayment = doc_loan["extraPayment"];
            auto pParamsAccPay = std::make_unique<fisim::loan::AcceleratedPay::Params>();
            auto& paramsAccPay = *pParamsAccPay;
            paramsAccPay.nMonthsStartOffset = paramsController.nMonthsOffset;

            paramsAccPay.minAmount = doc_extraPayment["min"].GetDouble();
            paramsAccPay.annualAllowanceFactor = doc_extraPayment["annualAllowanceFactor"].GetFloat();
            paramsAccPay.isFactorOfStartingAmount = true;
            paramsAccPay.isAnnualReset = true;

            doc_memberItr = doc_extraPayment.FindMember("initialUnusedAllowance");
            if (doc_memberItr != doc_extraPayment.MemberEnd()) {
                paramsAccPay.thisYearRemaining = doc_memberItr->value.GetDouble();
            } else {
                paramsAccPay.thisYearRemaining = paramsHandler.loan.amount * paramsAccPay.annualAllowanceFactor;
            }

            const auto& doc_calenderMonthsAllowed = doc_extraPayment["calenderMonthsAllowed"];
            assert(doc_calenderMonthsAllowed.Size() == 12u);
            std::array<Bool, 12> calenderMonthsAllowed;
            for (auto i = 0u; i < 12u; ++i) {
                calenderMonthsAllowed[i] = doc_calenderMonthsAllowed[i].GetBool();
            }
            paramsAccPay.monthsAllowed = calendarToSimMonths(calenderMonthsAllowed, monthInit, nMonths);

            // ParamsPayoff
            auto pParamsPayoff = std::make_unique<fisim::loan::LoanPayoff::Params>();
            auto& paramsPayoff = *pParamsPayoff;
            paramsPayoff.isFixedNmonthsPenalty = true;
            paramsPayoff.nMonthsThresh = std::numeric_limits<tUint>::max();
            paramsPayoff.repaymentOption = paramsHandler.settings.repaymentOption;
            paramsPayoff.monthlyPayment = paramsHandler.settings.monthlyPayment;

            // We expect either penaltyFactor or nMonthsPenalty
            doc_memberItr = doc_loan.FindMember("penaltyFactor");
            if (doc_memberItr != doc_loan.MemberEnd()) {
                paramsPayoff.nMonthsPenalty = 0u;
                paramsPayoff.penaltyFactor = doc_memberItr->value.GetFloat();
            } else {
                paramsPayoff.nMonthsPenalty = doc_loan["nMonthsPenalty"].GetUint();
                paramsPayoff.penaltyFactor = 0.0;
            }

            parseResults.backdoors[i].pMonthsAggAllowed = monthsStrategyAllowed.data();
            parseResults.loansData.push_back(std::make_unique<LoanControllerAgg>(
                std::move(pParamsController), std::move(pParamsHandler), std::move(pParamsAccPay), std::move(pParamsPayoff), std::move(monthsStrategyAllowed)));
        } else if (repaymentStrategy == 2u) {
            // ParamsHoliday
            const auto& doc_holiday = doc_loan["holiday"];
            auto pParamsHoliday = std::make_unique<fisim::loan::LoanHoliday::Params>();
            auto& paramsHoliday = *pParamsHoliday;

            paramsHoliday.nMonthsTermRemaining = nMonths;
            paramsHoliday.nMonthsYearRemaining = paramsHoliday.nMonthsPerYear;
            paramsHoliday.nMonthsStartOffset = paramsController.nMonthsOffset;
            paramsHoliday.pLoan = &pParamsHandler->loan;
            paramsHoliday.pNumMonthsToEnd = &paramsController.nMonthsToEnd;

            paramsHoliday.nMonthsPerYear = doc_holiday["nMonthsPerYear"].GetUint();
            doc_memberItr = doc_holiday.FindMember("initialUnusedAllowance");
            if (doc_memberItr != doc_holiday.MemberEnd()) {
                paramsHoliday.nMonthsYearRemaining = doc_memberItr->value.GetUint();
            }

            doc_memberItr = doc_holiday.FindMember("isSequenceStarted");
            if (doc_memberItr != doc_holiday.MemberEnd()) {
                paramsHoliday.isSequenceStarted = doc_memberItr->value.GetBool();
            }

            paramsHoliday.isAllowConsecutive = doc_holiday["shouldAllowConsecutive"].GetBool();
            paramsHoliday.isExtendTerm = doc_holiday["shouldExtendTerm"].GetBool();

            parseResults.loansData.push_back(std::make_unique<LoanControllerConsv>(std::move(pParamsController), std::move(pParamsHandler), std::move(pParamsHoliday)));
        } else {
            parseResults.loansData.push_back(std::make_unique<LoanControllerNor>(std::move(pParamsController), std::move(pParamsHandler)));
        }

    } // for

    return parseResults;
}

TaxHandler parseTaxHandler(JsonDoc const& document, CalendarMonth monthInit) {
    std::vector<TaxAccount> taxAccounts;

    auto const& doc_taxAccounts = document["taxAccounts"].GetArray();
    taxAccounts.resize(doc_taxAccounts.Size());

    for (auto i = 0u; i < doc_taxAccounts.Size(); ++i) {
        auto const& doc_account = doc_taxAccounts[i];
        TaxAccount& account = taxAccounts[i];

        // set the state
        auto const& doc_state = doc_account["taxYearStartingState"];
        account.state.earnings = doc_state["earnings"].GetDouble();
        account.state.deductions = doc_state["deductions"].GetDouble();
        account.state.taxPaid = doc_state["taxPaid"].GetDouble();

        auto itr = doc_account.FindMember("nextTaxReturnAmount");
        if (itr != doc_account.MemberEnd()) {
            account.state.amountTaxReturn = itr->value.GetDouble();
            account.state.taxRate = doc_account["nextTaxRate"].GetFloat();
        }

        auto pConfig = std::make_unique<Config>();
        Config& config = *pConfig;

        config.id.idTax = doc_account["id"].GetInt();
        config.taxYearStartMonth = static_cast<CalendarMonth>(doc_account["taxYearStartMonth"].GetUint());
        config.taxReturnMonth = static_cast<CalendarMonth>(doc_account["taxReturnMonth"].GetUint());
        config.isTaxLossCarriedForward = doc_account["isTaxLossCarriedForward"].GetBool();

        auto const& doc_incomeAmounts = doc_account["incomeAmountsArray"];
        auto arrSize = doc_incomeAmounts.Size();
        config.progTaxDataPoints.incomeAmounts.resize(arrSize);
        for (auto i = 0u; i < arrSize; ++i) {
            config.progTaxDataPoints.incomeAmounts[i] = doc_incomeAmounts[i].GetDouble();
        }

        auto const& doc_taxRates = doc_account["taxRatesArray"];
        config.progTaxDataPoints.taxRates.resize(arrSize);
        for (auto i = 0u; i < arrSize; ++i) {
            config.progTaxDataPoints.taxRates[i] = doc_taxRates[i].GetFloat();
        }

        auto const annualDeduction = doc_account["annualDeduction"].GetFloat();

        auto pParamsAmount = std::make_unique<UncertainAmount::Params>();
        auto& paramsAmount = *pParamsAmount;
        paramsAmount.amountMin = annualDeduction;
        paramsAmount.amountMax = annualDeduction;
        paramsAmount.changeFactorMin = 1.0;
        paramsAmount.changeFactorMax = 1.0;
        paramsAmount.changeInterval = 12u;
        paramsAmount.intervalElapsedInit = calMonthsDiff(config.taxYearStartMonth, monthInit);

        config.amountDeductible = UncertainAmount(std::move(pParamsAmount));

        account.configure(std::move(pConfig), monthInit);
    }

    TaxHandler taxHandler{std::move(taxAccounts), monthInit};

    return taxHandler;
}

void monthlyRecordSerializer(MonthlyRecords const& monthlyRecords, Writer<StringBuffer>& writer) {
    writer.StartObject();

    writer.Key("monthlyRecords");
    writer.StartArray();
    auto nMonths = monthlyRecords.size();
    for (auto indMonth = 0u; indMonth < nMonths; indMonth++) {
        auto const& monthRecord = monthlyRecords[indMonth];

        writer.StartObject();

        writer.Key("balance");
        writer.Double(monthRecord.balance);

        writer.Key("transactions");
        writer.StartArray();
        for (auto indTransac = 0u; indTransac < monthRecord.transacs.size(); indTransac++) {
            auto const& transac = monthRecord.transacs[indTransac];

            writer.StartObject();
            if (transac.id.idCashflow != 0) {
                writer.Key("cashflowId");
                writer.Int(transac.id.idCashflow);
            }
            if (transac.id.idLoan != 0) {
                writer.Key("loanId");
                writer.Int(transac.id.idLoan);
            }
            if (transac.id.idTax != 0) {
                writer.Key("taxAccountId");
                writer.Int(transac.id.idTax);
            }

            writer.Key("amount");
            writer.Double(transac.amount);

            if (transac.description != "") {
                writer.Key("description");
                writer.String(transac.description.c_str(), static_cast<unsigned>(transac.description.size()));
            }

            writer.EndObject();
        }
        writer.EndArray();

        writer.EndObject();
    }
    writer.EndArray();

    writer.EndObject();
}

} // namespace json
} // namespace fisim

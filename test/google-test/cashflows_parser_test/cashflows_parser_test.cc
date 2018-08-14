#include "json_helpers.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "simulator.h"
#include <gtest/gtest.h>

namespace {

TEST(ParserCashflowsTest, test00) {
    using namespace rapidjson;
    FILE* fp = std::fopen("./test/google-test/cashflows_parser_test/cashflow_items_test00.json", "r"); // non-Windows use "r"

    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document document;
    document.ParseStream(is);
    fclose(fp);

    std::vector<fisim::tax::TaxAccount> taxAccounts;
    taxAccounts.resize(3);

    auto pConfig0 = std::make_unique<fisim::tax::TaxAccount::Config>();
    pConfig0->id.idTax = 1;
    pConfig0->taxReturnMonth = fisim::CalendarMonth::jan;
    pConfig0->taxYearStartMonth = fisim::CalendarMonth::oct;
    taxAccounts[0].configure(std::move(pConfig0), fisim::CalendarMonth::apr);

    auto pConfig1 = std::make_unique<fisim::tax::TaxAccount::Config>();
    pConfig1->id.idTax = 5;
    pConfig1->taxReturnMonth = fisim::CalendarMonth::jan;
    pConfig1->taxYearStartMonth = fisim::CalendarMonth::oct;
    taxAccounts[1].configure(std::move(pConfig1), fisim::CalendarMonth::apr);

    auto pConfig2 = std::make_unique<fisim::tax::TaxAccount::Config>();
    pConfig2->id.idTax = 2;
    pConfig2->taxReturnMonth = fisim::CalendarMonth::jan;
    pConfig2->taxYearStartMonth = fisim::CalendarMonth::oct;
    taxAccounts[2].configure(std::move(pConfig2), fisim::CalendarMonth::apr);

    fisim::tax::TaxHandler taxHandler{std::move(taxAccounts), fisim::CalendarMonth::dec};

    const fisim::json::LinkToTaxAccount linkToTaxAccount{taxHandler};

    const auto relations = fisim::json::parseRelationsRecords(document);

    auto cashflows = fisim::json::parseCashflows(document, relations.cashflowTaxLinks, linkToTaxAccount);

    EXPECT_EQ(true, true); // manually compare private data with the json file
}

} // namespace

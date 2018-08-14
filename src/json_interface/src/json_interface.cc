#include "json_interface.h"
#include "json_helpers.h"
#include "sim_manager.h"
#include <cassert>

namespace fisim {
namespace json {

using simulation::SimManager;
using simulation::Simulator;

MonthlyRecords runSim(JsonDoc const& doc) {
    const tUint nMonths = doc["duration"].GetUint();
    CalendarMonth monthInit = static_cast<CalendarMonth>(doc["startDate"]["month"].GetUint());
    tAmount balanceInit = doc["balanceInit"].GetDouble();
    tAmount balanceReserve = doc["balanceReserve"].GetDouble();
    assert(!(balanceReserve < 0.0));
    assert(balanceInit > balanceReserve);

    Simulator::SimComponents simComponents;
    SimManager::SimBackdoor simBackdoor;

    const auto relations = parseRelationsRecords(doc);

    simComponents.taxHandler = parseTaxHandler(doc, monthInit);
    const LinkToTaxAccount linkToTaxAccount{simComponents.taxHandler};

    simComponents.cashflows = parseCashflows(doc, relations.cashflowTaxLinks, linkToTaxAccount);
    auto loansParseResult = parseLoans(doc, relations.loanTaxLinks, monthInit, nMonths);
    simComponents.loanControllers = std::move(loansParseResult.loansData);
    simBackdoor.loansBackdoors = std::move(loansParseResult.backdoors);

    Simulator simulator{std::move(simComponents)};
    SimManager simManager{std::move(simBackdoor), simulator};

    simManager.simulate(nMonths, balanceInit, balanceReserve);

    return simulator.releaseRecords();
}

ResultSummary runSim(JsonDoc const& doc, Writer<StringBuffer>& writer) {
    auto monthlyRecords = runSim(doc);

    monthlyRecordSerializer(monthlyRecords, writer);

    ResultSummary summary;

    summary.nMonths = monthlyRecords.size();
    summary.balanceEnd = monthlyRecords.back().balance;

    return summary;
}

} // namespace json
} // namespace fisim

#ifndef FISIM_JSON_INTERFACE_HELPERS_H_
#define FISIM_JSON_INTERFACE_HELPERS_H_

#include "cashflow.h"
#include "loan_controller.h"
#include "sim_manager.h"
#include "tax_handler.h"
#include "transacs_recorder.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <unordered_map>

namespace fisim {
namespace json {

using cashflow::Cashflow;
using LoanBackdoor = simulation::SimManager::SimBackdoor::LoanBackdoor;
using loan::LoanController;
using MonthlyRecords = transacs::TransacsRecorder::MonthlyRecords;
using tax::TaxHandler;
using JsonDoc = rapidjson::GenericValue<rapidjson::UTF8<>>;
using rapidjson::StringBuffer;
using rapidjson::Writer;

struct CashflowTaxLink {
    tInt taxAccountId = 0;
    bool shouldWithhold = false;
    tFloat currentRate = 0.0;
};

using CashflowTaxLinkLookupMap = std::unordered_map<tInt, CashflowTaxLink>; ///< Cashflow ID to tax link lookup
using LoanTaxLinkLookupMap = std::unordered_map<tInt, tInt>;                ///< Loan ID to tax account ID lookup

struct RelationsLookup {
    CashflowTaxLinkLookupMap cashflowTaxLinks;
    LoanTaxLinkLookupMap loanTaxLinks;
};

struct LoansParseResult {
    std::vector<std::unique_ptr<LoanController>> loansData; // Using pointers because LoanController is an abstract class
    std::vector<LoanBackdoor> backdoors;
};

/// 
/// @brief Links the cashflow to it's tax account
/// 
class LinkToTaxAccount {
  public:
    LinkToTaxAccount(const TaxHandler& taxHandler);

    void operator()(tInt taxId, Cashflow::TaxInfo::NextUpdate& o_taxUpdate) const;

  private:
    const TaxHandler* m_pTaxHandler;
};

RelationsLookup parseRelationsRecords(JsonDoc const& document);

std::vector<Cashflow> parseCashflows(JsonDoc const& document, const CashflowTaxLinkLookupMap& taxLinkLookupMap, LinkToTaxAccount linkToTaxAccount);

LoansParseResult parseLoans(JsonDoc const& document, const LoanTaxLinkLookupMap& taxLinkLookupMap, CalendarMonth monthInit, tUint nMonths);

void monthlyRecordSerializer(MonthlyRecords const& monthlyRecords, Writer<StringBuffer>& writer);

TaxHandler parseTaxHandler(JsonDoc const& document, CalendarMonth monthInit);

} // namespace json
} // namespace fisim

#endif /* FISIM_JSON_INTERFACE_HELPERS_H_ */

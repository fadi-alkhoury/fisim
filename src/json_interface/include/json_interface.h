#ifndef INCLUDE_FISIMLIB_JSON_INTERFACE_H_
#define INCLUDE_FISIMLIB_JSON_INTERFACE_H_
#define RAPIDJSON_HAS_STDSTRING 1

#include "transacs_recorder.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace fisim {
namespace json {

using rapidjson::StringBuffer;
using rapidjson::Writer;
using JsonDoc = rapidjson::GenericValue<rapidjson::UTF8<>>;
using MonthlyRecords = transacs::TransacsRecorder::MonthlyRecords;

struct ResultSummary {
    tSize nMonths;      ///< Number of simulated months.
    tAmount balanceEnd; ///< Ending balance.
};

///
/// Runs the sim specified in the JSON and returns the monthlyRecords.
///
/// \param[in] doc  JSON specifying the simulation scenario.
///
/// \return List of transactions for all simulation months.
///
MonthlyRecords runSim(JsonDoc const& doc);

///
/// Runs the sim specified in the JSON and writes the transactions to a JSON string.
///
/// \param[in] doc     JSON specifying the simulation scenario.
/// \param[in] writer  Writer to be used for writing the JSON string.
///
/// \return  A summary of the results.
///
ResultSummary runSim(JsonDoc const& doc, Writer<StringBuffer>& writer);

} // namespace json
} // namespace fisim

#endif /* INCLUDE_FISIMLIB_JSON_INTERFACE_H_ */

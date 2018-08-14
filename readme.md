
# Fisim: Financial simulator

`Fisim` simulates financial scenarios and generates monthly transactions for the simulation period.

Live demo with a web front-end: https://fadialkhoury.com/fisim

## Input

A financial scenario can be specified in terms of cashflow, loan, and tax elements. The library provides a JSON interface for specifying scenarios. See [interface_in.ts](interface_in.ts) for the json input interface specification. 

## Output

`Fisim` outputs the simulated transactions and the ending balance for all the simulated months.

The library provides a JSON output interface, as well as a C++ output interface. See [interface_out.ts](interface_out.ts) for the json output interface specification. The C++ output interface contains the same information but with C++ types. 

---
## Installing the library
### Installing dependencies

Fisim uses [rapidjson](https://github.com/Tencent/rapidjson) for JSON parsing and generation.
```
git clone https://github.com/Tencent/rapidjson.git
cp -r rapidjson/include /usr/local/
```

Fisim also uses the open-source mixed integer linear programming solver [Cbc](https://github.com/coin-or/Cbc) for finding feasible plans. 

As of writing, the original Cbc library lacks CMake support. You can use this [Cbc port with CMake support](https://github.com/fadi-alkhoury/coin-or-cbc-with-cmake.git) instead. Clone and install the Cbc port as described in the [readme](https://github.com/fadi-alkhoury/coin-or-cbc-with-cmake#readme).

### Installing Fisim

With the dependencies installed, you can now clone this repository and install `Fisim`.

```
git clone https://github.com/fadi-alkhoury/fisim.git
cd fisim
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make 
make test
make install
```

---

## How to use within your code

Linking to the library
```
find_package(fisim REQUIRED)

target_link_libraries(${YOUR_TARGET_NAME} PRIVATE
  FisimJson)
```

### Running a simulation and getting the results as a JSON string
```
#include <equitysim/json_interface.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <fstream>
#include <string>

// ...

// Read the input JSON
// see `interface_in.ts` for the json input interface specification
std::ifstream fStreamJson{"./path/to/your/scenario/file.json"};
const std::string str{std::istreambuf_iterator<char>(fStreamJson), std::istreambuf_iterator<char>()};
rapidjson::Document scenarioJson;
scenarioJson.Parse(str);

// Create a writer for the JSON output
rapidjson::StringBuffer simOutBuffer;
Writer<StringBuffer> rapidjson::writer{simOutBuffer};

// Run the simulation and access results
const auto resultSummary = equitysim::json::runSim(scenarioJson, writer);

const auto numSimulatedMonths = resultSummary.nMonths;

// access the json string output
// see `interface_out.ts` for the json output interface specification
const char* pResult = simOutBuffer.GetString();

// ...

```

### Running a simulation and getting the results as a C++ type
```
#include <equitysim/json_interface.h>
#include <rapidjson/document.h>
#include <fstream>
#include <string>

// ...

// Read the input JSON
// see `interface_in.ts` for the json input interface specification
std::ifstream fStreamJson{"./path/to/your/scenario/file.json"};
const std::string str{std::istreambuf_iterator<char>(fStreamJson), std::istreambuf_iterator<char>()};
rapidjson::Document scenarioJson;
scenarioJson.Parse(str);

// Run the simulation and access results
const auto monthlyRecords = fisim::json::runSim(scenarioJson);

const auto numSimulatedMonths = monthlyRecords.size();
const auto endingBalance = monthlyRecords.back().balance;
const auto& lastMonthTransactions = monthlyRecords.back().transacs;
const auto lastTransaction = lastMonthTransactions.back();  // contains the amount, and the connections to a cashflow, tax account or loan, as well as a description.

// ...

```

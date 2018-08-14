interface Scenario {
  startDate: {
    month: number; ///< Calendar month 1-12
    year: number;
  };
  
  duration: number;        ///< Duration of the sim. At least 1 (nothing to simulate if 0)
  balanceInit: number;     ///< Initial balance (positive).
  balanceReserve: number;  ///< Reserve balance. The simulation will be stopped if the balance falls below this value (non-negative, less than balanceInit).

  cashflows: Array<Cashflow>;               ///< Cashflows for the scenario
  loans: Array<Loan>;                       ///< Loans for the scenario
  taxAccounts: Array<TaxAccount>;           ///< Tax accounts for the scenario
  relationsRecords: Array<RelationRecord>;  ///< Relationships between simulation elements (eg. how cashflows and loans are linked to tax accounts).
}

interface Cashflow {
  id: number;                  ///< Unique 32 bit integer
  
  stepsActive: Array<boolean>; ///< Array of booleans specifying if the cashflow is active for all the simulation months

  /// The cashflow can either be specified using one of the options below
  /// Option 1: Fixed amount
  amount?: number;

  /// Option 2: Uncertain amount
  amountMin?: number; ///< Minimum base amount
  amountMax?: number; ///< Maximum base amount
  changeInterval?: number; ///< With each interval, the base amount is multiplied by a factor within change factor bounds
  changeFactorMin?: number; ///< The minimum multiplication factor for each change
  changeFactorMax?: number; ///< The maximum multiplication factor for each change
  intervalElapsed?: number; ///< The number of elapsed months in the interval at the start of the sim. Can be negative.
  lossProbability?: number; ///< Probability (0 to 1) of completely loosing the cashflow in any month.
}

interface Loan {
  id: number; ///< Unique 32 bit integer

  amount: number;            ///< loan amount
  nMonthsToStart: number;    ///< number of months from the start of the sim until the start of the loan (0 -> start at the same time. Can be negative)
  duration: number;          ///< duration of the loan in months
  repaymentStrategy: number; ///< 0: normal, 1: aggressive, 2: conservative

  repaymentOption: number; ///< 0: interest-only, 1: duration-specified (fixed amortization), 2: amount-specified (monthly payment)
  monthlyPayment?: number; ///< [Optional] Monthly payment for the amount-specified or the duration-specified options
  isForceRepay?: boolean;  //only for options 0 and 2, and if the duration falls within the simulation

  /// Month and interest data points. Months are simulation months: month 0 --> first sim month
  /// The intervals between data points can be obtained by interpolation
  interestMonthsArray: Array<number>;
  interestRatesArray: Array<number>;

  monthlyPaymentMax?: number; ///< [Optional] The maximum monthly payment in case of variable interest

  /// Fractions for a partial first month
  firstMonthInterestFraction: number;
  firstMonthRepaymentFraction?: number; ///< [Optional] Only needed if repaymentOption is 1 or 2 (amount-specified or the duration-specified )

  extraPayment?: LoanExtraPaymentConfig;
  nMonthsPenalty?: number; ///< [Only relevant if penaltyFactor is undefined]  The penalty as amortized interest for a number of months.
  penaltyFactor?: number;  ///< [Only relevant if nMonthsPenalty is undefined] The penalty is a percent of the remaining principle, instead of months of interest.

  holiday?: LoanHolidayConfig;
}

interface LoanExtraPaymentConfig {
  min: number;                           ///< minimum per extra payment
  annualAllowanceFactor: number;         ///< Maximum annual amount as a portion of initial amount
  initialUnusedAllowance?: number;       ///< [Only relevant for pre-existing loans] The usable allowance in the first month of simulating the loan (sim month 0 or the start of the loan if it falls after sim start)
  calenderMonthsAllowed: Array<boolean>; ///< Allowed or disallowed for months Jan to Dec
}

interface LoanHolidayConfig {
  nMonthsPerYear: number;          ///< Allowed holiday months per year
  initialUnusedAllowance?: number; ///< [Only relevant for Loan::nMonthsToStart < 0] The usable allowance of holiday months in the first month of simulating the loan (sim month 0 or the start of the loan if it falls after sim start)
  isSequenceStarted?: boolean;     ///< [Only relevant for Loan::nMonthsToStart < 0] True: a holiday was taken in the month before the sim month
  shouldAllowConsecutive: boolean; ///< Consecutive holiday months are allowed
  shouldExtendTerm: boolean;       ///< The loan term is extended when taking a holiday
}

interface TaxAccount {
  id: number; ///< Unique 32 bit integer

  taxYearStartMonth: number; ///< Calendar month when the tax year starts (1-12)

  /// Progressive tax curve
  incomeAmountsArray: Array<number>;
  taxRatesArray: Array<number>;

  annualDeduction: number;
  isTaxLossCarriedForward: boolean;
  taxReturnMonth: number; ///< Calendar month when the tax return transaction is made

  /// Parameters required if starting after the start of tax year but before the tax return month
  nextTaxReturnAmount?: number;
  nextTaxRate?: number;

  /// The status of the ongoing tax year, or the tax year just completed in case of starting in a new tax year
  taxYearStartingState: {
    earnings: number;
    deductions: number;
    taxPaid: number;
  };
}

///
/// A RelationRecord contains the linkage information for simulation elements. The records have to follow the requirements described here.
/// Rules:
/// - Exactly two IDs should be present in a record.
/// - A cashflow can be linked to only one tax account (no conflicting assignments between 2 records).
/// - A Loan can be linked to only one tax account (no conflicting assignments between 2 records).
///
interface RelationRecord {
  cashflowId?: number;              ///< The ID of the cashflow to link with a tax account.
  loanId?: number;                  ///< The ID of the loan to link with a tax account.
  taxAccountId?: number;            ///< The ID of the tax account.
  relationData?: CashflowTaxConfig; ///< Optional relation link data. (Required only when linking a cashflow to a tax account.)
}

///
/// Describes how a cashflow is connected to a tax account.
///
interface CashflowTaxConfig {
  shouldWithhold?: boolean; ///< [Optional, only for incomes] Whether or not the tax is withheld for the income.

  /// The withholding tax rate, or deduction rate at the start of sim. Tax rates update after a tax return.
  /// This value is not relevant if the cashflow starts after the first tax return month,
  /// but a value is currently provided nonetheless, because ensuring consistency was
  /// not easy in the front-end.
  currentRate?: number;
}

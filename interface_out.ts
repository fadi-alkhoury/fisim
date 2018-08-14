///
/// The simulation results for the simulated months.
///
interface SimulationResult {
  monthlyRecords: Array<MonthRecord>;
}

///
/// The transactions and end balance for a simulated month.
///
interface MonthRecord {
  transactions: Array<Transaction>; ///< The simulated transactions.
  balance: number;                  ///< Balance at end of month.
}

///
/// Specifies the data of a transaction.
///
interface Transaction {

  ///
  /// Transaction amount. Positive: incoming, Negative: outgoing.
  ///
  amount: number;

  ///
  /// The optional description field contains further information:
  ///  - cashflowId + description
  ///      "loss"                  : The cashflow was lost
  ///  - loanId + description
  ///      "credit"                : The credited is added to the balance
  ///      "interest"              : Interest component of the monthly payment
  ///      "repayment"             : Repayment component of the monthly payment
  ///      "extra_repayment"       : An extra repayment
  ///
  description?: string; ///< An optional description.

  ///
  /// Transaction amount with link to simulation entities.
  /// At least one entity should be provided
  ///  - cashflowId                : An income or cost, before any tax.
  ///  - cashflowId + taxAccountId : Tax payment for the income
  ///  - taxAccountId              : Tax return
  ///  - loanId                    : A loan payment.
  ///
  cashflowId?: number;   ///< ID of the cashflow, in case the transaction is related to a cashflow.
  loanId?: number;       ///< ID of the loan, in case the transaction is related to a loan.
  taxAccountId?: number; ///< ID of the tax account, if the transaction is a tax payment.
}

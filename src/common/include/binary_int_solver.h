#ifndef FISIM_BINARY_INT_SOLVER_H_
#define FISIM_BINARY_INT_SOLVER_H_

#include <coin/OsiClpSolverInterface.hpp>
#include <coin/CbcSolver.hpp>
#include <vector>


namespace fisim {
namespace utils {

///
/// Tries to find a combination of values with the minimum sum above the threshold.
/// 	Returns either:
/// 		- the global minimum if it could be proven within the time limit, OR
/// 		- a feasible combination if one is found but could not be proven within the time limit, OR
/// 		- an empty combination if no feasible combination was found within the time limit
///
template <typename T> //numerical type
std::vector<size_t> findFeasibleComb(std::vector<T> const& values, T threshold) {
	std::vector<size_t> combination;

	const int nRhsCols = static_cast<int>(values.size());
	constexpr double POSTIVE_INF  = std::numeric_limits<double>::max();

	// sum(x_i * values_i) >= threshold  =>  threshold <= sum(x_i * values_i) <= POSTIVE_INF
	// x_i is more naturally bool for our specific problem, but x_i is real for the general MIP problem the solver takes.
    std::vector<double> lhsLowerBounds, lhsUpperBounds;
    lhsLowerBounds.resize(nRhsCols, 0.0);
    lhsUpperBounds.resize(nRhsCols, 1.0);

	double constraintLowerBounds[1u], constraintUpperBounds[1u];
	constraintLowerBounds[0] = threshold; //implicit cast to double
	constraintUpperBounds[0] = POSTIVE_INF;

	//Constraints matrix values_i
	CoinPackedMatrix matrix {false, 0, 0};
	CoinPackedVector constraintCoefs;
	matrix.setDimensions(0, nRhsCols);
	constraintCoefs.reserve(nRhsCols);
	for(int i = 0; i < nRhsCols; ++i) {
		constraintCoefs.insert(i, values[i]); //implicit cast to double
	}
	matrix.appendRow(constraintCoefs); //copied

	//load the problem to OSI:  minimize sum(x_i * values_i)
	OsiClpSolverInterface si;
	si.loadProblem(
		matrix,
		lhsLowerBounds.data(),
		lhsUpperBounds.data(),
		values.data(),
		constraintLowerBounds,
		constraintUpperBounds
	);

	std::vector<int> indices;
	indices.reserve(nRhsCols);
	for(int i = 0; i < nRhsCols; ++i) {
		indices[i] = i;
	}
	si.setInteger(indices.data(), nRhsCols);

	CbcModel model {si};
	model.setMaximumSeconds(10);
	model.setLogLevel(0);

	model.initialSolve();
	model.branchAndBound();

	// Check the solution
	const double* solution = model.bestSolution();
	if (solution != nullptr) {
		combination.reserve(nRhsCols);

		for(int i = 0; i < nRhsCols; ++i) {
			if(solution[i] == 1.0) {
				combination.emplace_back(i);
			}
		}
	}

	return combination;
}

}} /* namespaces */

#endif /* FISIM_BINARY_INT_SOLVER_H_ */

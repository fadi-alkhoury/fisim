#include "binary_int_solver.h"
#include "coin/OsiCbcSolverInterface.hpp"

#include <array>
#include <gtest/gtest.h>
#include <vector>

TEST(CBC, simpleBAB) { // problem from simpleBAB.cpp
    OsiClpSolverInterface model;
    model.messageHandler()->setLogLevel(0);

    CoinBigIndex start[] = {0, 1, 2};
    int index[] = {0, 0};
    double values[] = {1.0, 2.0};
    double collb[] = {0.0, 0.0};
    double colub[] = {10.0, 10.0};
    double obj[] = {1.0, 1.0};
    double rowlb[] = {0.0};
    double rowub[] = {3.9};

    // obj: Max x0 + x1
    //  st. x0 + 2 x1 <= 3.9
    //          0 <= x0 <= 10 and integer
    //          0 <= x1 <= 10
    model.loadProblem(2, 1, start, index, values, collb, colub, obj, rowlb, rowub);
    model.setInteger(0); // change to overload that takes inds
    model.setObjSense(-1.0);

    CbcModel model2(model);
    model2.branchAndBound();
    // optimal = model2.isProvenOptimal();
    const double* val = model2.getColSolution(); // x0 = 3, x1 = 0.45
    printf("Solution %g %g\n", val[0], val[1]);

    EXPECT_DOUBLE_EQ(val[0], 3);
    EXPECT_DOUBLE_EQ(val[1], 0.45);
}

TEST(CBC, building) { // problem from Osi/examples/build.cpp
    int nRhsCols = 2; // unknown at compile time
    double const POSTIVE_INF = std::numeric_limits<double>::max();
    double const NEGATIVE_INF = std::numeric_limits<double>::min();

    // Bounds of x_i and the right hand sides of constraints
    std::vector<double> lhsLowerBounds, lhsUpperBounds;
    lhsLowerBounds.resize(nRhsCols, 0.0);
    lhsUpperBounds.resize(nRhsCols, POSTIVE_INF);

    double constraintsLowerBounds[2u], constraintsUpperBounds[2u]; // bounds for 2 rows
    constraintsLowerBounds[0] = NEGATIVE_INF;
    constraintsLowerBounds[1] = NEGATIVE_INF;
    constraintsUpperBounds[0] = 3.0;
    constraintsUpperBounds[1] = 3.0;

    // x_i constraint matrix
    CoinPackedMatrix matrix{false, 0, 0};
    matrix.setDimensions(0, nRhsCols);

    // 1 x0 + 2 x1 <= 3  =>  -infinity <= 1 x0 + 2 x2 <= 3
    CoinPackedVector lhsRow;
    lhsRow.reserve(nRhsCols);
    lhsRow.insert(0, 1.0);
    lhsRow.insert(1, 2.0);
    matrix.appendRow(lhsRow); // copied

    // 2 x0 + 1 x1 <= 3  =>  -infinity <= 2 x0 + 1 x1 <= 3
    lhsRow.setElement(0, 2.0);
    lhsRow.setElement(1, 1.0);
    matrix.appendRow(lhsRow);

    // minimize -1 x0 - 1 x1
    std::vector<double> ojectiveCoefs;
    ojectiveCoefs.resize(nRhsCols, -1.0);

    // load the problem to OSI
    OsiCbcSolverInterface si;
    si.messageHandler()->setLogLevel(0);
    si.loadProblem(matrix, lhsLowerBounds.data(), lhsUpperBounds.data(), ojectiveCoefs.data(), constraintsLowerBounds, constraintsUpperBounds);

    int indices[2] = {0, 1};
    si.setInteger(indices, 2);
    si.setMaximumSeconds(10.0);

    si.branchAndBound();

    // Check the solution
    if (si.isProvenOptimal()) {
        const double* solution = si.getColSolution();

        EXPECT_DOUBLE_EQ(si.getObjValue(), -2);
        EXPECT_DOUBLE_EQ(solution[0], 1);
        EXPECT_DOUBLE_EQ(solution[1], 1);

    } else {
        std::cout << "Didn't find optimal solution." << std::endl;
        // Could then check other status functions.
        EXPECT_EQ(0, 1);
    }
}

TEST(CBC, binaryInteger20) {
    std::vector<double> values{68.04, 45.89, 44.22, 65.25, 12.25, 4.06, 0.00, 77.52, 53.73, 89.25, 110.36, 55.47, 0.30, 277.52, 453.73, 39.25, 25.47, 4.30, 77.52, 153.73};

    std::array<size_t, 3u> desiredInds{5u, 9u, 17u};
    double target = 0.0;
    for (size_t ind : desiredInds) {
        target += values[ind];
    }
    double threshold = target - 0.001;

    std::vector<size_t> resultInds = fisim::utils::findFeasibleComb(values, threshold);

    size_t nIndsResult = resultInds.size();
    EXPECT_EQ(nIndsResult, 3u);
    for (size_t i = 0u; i < nIndsResult; ++i) {
        EXPECT_EQ(resultInds[i], desiredInds[i]);
    }
}

TEST(CBC, binaryInteger100) {
    std::vector<double> values{68.04,  45.89,  44.22,  65.25,  12.25,  4.06,   0.00,   77.52,  53.73,  89.25,  110.36, 55.47,  0.30,   277.52, 453.73, 39.25, 25.47,
                               4.30,   77.52,  153.73, 168.04, 5.89,   24.22,  63.25,  12.25,  44.06,  60.00,  57.52,  13.73,  189.25, 140.36, 52.47,  10.30, 227.52,
                               153.73, 30.25,  125.47, 49.30,  97.52,  15.73,  18.04,  75.89,  124.22, 93.25,  312.25, 54.06,  90.00,  51.52,  213.73, 89.25, 40.36,
                               92.47,  10.30,  327.52, 53.73,  80.25,  105.47, 49.30,  75.52,  152.73, 68.04,  25.89,  29.22,  53.25,  892.25, 464.06, 80.00, 57.52,
                               36.73,  792.25, 340.36, 652.47, 65.30,  27.52,  153.73, 300.25, 126.47, 429.30, 74.52,  145.73, 60.36,  59.47,  18.30,  37.52, 23.73,
                               85.25,  195.47, 69.30,  55.52,  512.73, 98.04,  95.89,  33.22,  550.25, 82.25,  44.06,  585.00, 67.52,  96.73,  79.25};

    // sol exists
    std::array<size_t, 8u> desiredInds{5u, 9u, 17u, 22u, 55u, 75u, 88u, 92u};
    double target = 0.0;
    for (size_t ind : desiredInds) {
        target += values[ind];
    }
    double threshold = target - 0.001;

    std::vector<size_t> resultInds = fisim::utils::findFeasibleComb(values, threshold);

    double result = 0;
    for (size_t ind : resultInds) {
        result += values[ind];
    }
    EXPECT_DOUBLE_EQ(result, target);

    // sol does not exist
    resultInds = fisim::utils::findFeasibleComb(values, threshold * 100);

    EXPECT_EQ(resultInds.size(), 0u);
}

TEST(CBC, binaryInteger200) {
    std::vector<double> values{
        68.04,  45.89, 44.22,  65.25, 12.25,  4.06,   0.00,   77.52, 53.73,  89.25,  110.36, 55.47,  0.30,  277.52, 453.73, 39.25,  25.47,  4.30,   77.52, 153.73,
        168.04, 5.89,  24.22,  63.25, 12.25,  44.06,  60.00,  57.52, 13.73,  189.25, 140.36, 52.47,  10.30, 227.52, 153.73, 30.25,  125.47, 49.30,  97.52, 15.73,
        18.04,  75.89, 124.22, 93.25, 312.25, 54.06,  90.00,  51.52, 213.73, 89.25,  40.36,  92.47,  10.30, 327.52, 53.73,  80.25,  105.47, 49.30,  75.52, 152.73,
        68.04,  25.89, 29.22,  53.25, 892.25, 464.06, 80.00,  57.52, 36.73,  792.25, 340.36, 652.47, 65.30, 27.52,  153.73, 300.25, 126.47, 429.30, 74.52, 145.73,
        60.36,  59.47, 18.30,  37.52, 23.73,  85.25,  195.47, 69.30, 55.52,  512.73, 98.04,  95.89,  33.22, 550.25, 82.25,  44.06,  585.00, 67.52,  96.73, 79.25,
        68.04,  45.89, 44.22,  65.25, 12.25,  4.06,   0.00,   77.52, 53.73,  89.25,  110.36, 55.47,  0.30,  277.52, 453.73, 39.25,  25.47,  4.30,   77.52, 153.73,
        168.04, 5.89,  24.22,  63.25, 12.25,  44.06,  60.00,  57.52, 13.73,  189.25, 140.36, 52.47,  10.30, 227.52, 153.73, 30.25,  125.47, 49.30,  97.52, 15.73,
        18.04,  75.89, 124.22, 93.25, 312.25, 54.06,  90.00,  51.52, 213.73, 89.25,  40.36,  92.47,  10.30, 327.52, 53.73,  80.25,  105.47, 49.30,  75.52, 152.73,
        68.04,  25.89, 29.22,  53.25, 892.25, 464.06, 80.00,  57.52, 36.73,  792.25, 340.36, 652.47, 65.30, 27.52,  153.73, 300.25, 126.47, 429.30, 74.52, 145.73,
        60.36,  59.47, 18.30,  37.52, 23.73,  85.25,  195.47, 69.30, 55.52,  512.73, 98.04,  95.89,  33.22, 550.25, 82.25,  44.06,  585.00, 67.52,  96.73, 79.25};

    // sol exists
    std::array<size_t, 8u> desiredInds{5u, 9u, 17u, 22u, 55u, 75u, 188u, 192u};
    double target = 0.0;
    for (size_t ind : desiredInds) {
        target += values[ind];
    }
    double threshold = target - 0.001;

    std::vector<size_t> resultInds = fisim::utils::findFeasibleComb(values, threshold);

    double result = 0;
    for (size_t ind : resultInds) {
        result += values[ind];
    }
    EXPECT_DOUBLE_EQ(result, target);

    // sol does not exist
    resultInds = fisim::utils::findFeasibleComb(values, threshold * 100);

    EXPECT_EQ(resultInds.size(), 0u);
}

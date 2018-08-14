#############################################
# Unit tests
#############################################


# Add a testing executable
add_executable(optimization_test 
  binary_integer_programming_test.cc 
)

find_package(CoinOrCbc REQUIRED)  #only needed because we have tests that need libOsiCbc
target_link_libraries(optimization_test
    libOsiCbc
    fisim::common
    GTest::GTest 
    GTest::Main
)

add_test(test_all optimization_test)


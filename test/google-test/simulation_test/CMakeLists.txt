#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(simulation_test 
  simulation_test.cc 
  )

target_link_libraries(simulation_test
    fisim::json
    fisim::simulation
    fisim::common
    GTest::GTest 
    GTest::Main
)

add_test(test_all simulation_test)


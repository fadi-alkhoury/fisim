
SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra -Werror -pedantic -fno-rtti -fstack-protector-all -fvisibility=hidden")

#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(utils_parsing_test utils_parsing_test.cc)

target_link_libraries(utils_parsing_test
    fisim::json
    fisim::simulation
    GTest::GTest 
    GTest::Main
)

add_test(test_all utils_parsing_test)


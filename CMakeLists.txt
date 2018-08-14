cmake_minimum_required(VERSION 3.7.2)
project(fisim LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
	SET(CMAKE_CXX_FLAGS "-s -DNDEBUG -Ofast -Wall -Wextra -Werror -pedantic -fvisibility=hidden -Wconversion -Wno-sign-conversion")
	SET(CMAKE_C_FLAGS "-s -DNDEBUG -Ofast -Wall -Wextra -Werror -pedantic -fvisibility=hidden -Wconversion -Wno-sign-conversion")  
ELSE()
	SET(CMAKE_BUILD_TYPE "DEBUG")
	SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra -Werror -pedantic -fstack-protector-all -fvisibility=hidden -Wconversion -Wno-sign-conversion")
	SET(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra -Werror -pedantic -fstack-protector-all -fvisibility=hidden -Wconversion -Wno-sign-conversion")  
ENDIF() 
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# rtti is required by a dependency (coinorcbc)
#add_compile_options(-fno-rtti)  

# Add sub directories
add_subdirectory(src)

#############################################
# Unit tests
#############################################
add_subdirectory(test/google-test)

# enable CTest testing
enable_testing()


add_test(NAME common_test COMMAND common_test)
add_test(NAME optimization_test COMMAND optimization_test)
add_test(NAME loan_test COMMAND loan_test)

# Disabled because these need to be updated to new interfaces
#add_test(NAME cashflow_test COMMAND cashflow_test)
#add_test(NAME tax_test COMMAND tax_test)
#add_test(NAME transactions_test COMMAND transactions_test)

add_test(NAME utils_parsing_test COMMAND utils_parsing_test)
add_test(NAME tax_handler_parser_test COMMAND tax_handler_parser_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME cashflows_parser_test COMMAND cashflows_parser_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME loan_controllers_parser_test COMMAND loan_controllers_parser_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME simulation_test COMMAND simulation_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME monthly_records_serializer_test COMMAND monthly_records_serializer_test)


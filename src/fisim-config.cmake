# package configuration file


string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/fisim-${build_type}.cmake) 

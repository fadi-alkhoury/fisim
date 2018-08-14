set(TARGET_NAME cashflow)

# Add a library with the sources
add_library(${TARGET_NAME} STATIC 
  src/cashflow.cc)
  
add_library(fisim::cashflow ALIAS ${TARGET_NAME})

target_include_directories( ${TARGET_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/fisim> 
)

target_link_libraries(${TARGET_NAME}
    fisim::common
)

INSTALL(
  TARGETS ${TARGET_NAME}
  EXPORT  export_set
  DESTINATION lib
)

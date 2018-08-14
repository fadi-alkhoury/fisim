set(TARGET_NAME Fisim)

# Add a library with the sources
add_library(${TARGET_NAME} 
  src/sim_manager.cc
  src/simulator.cc)
  
add_library(fisim::simulation ALIAS ${TARGET_NAME})

target_include_directories( ${TARGET_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/fisim> 
)

target_link_libraries(${TARGET_NAME}
  fisim::loan
  fisim::cashflow
  fisim::transactions
  fisim::tax
)


# Install lib
INSTALL(
  TARGETS ${TARGET_NAME}
  EXPORT  export_set
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)




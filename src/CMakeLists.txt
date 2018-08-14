# Add sub directories
add_subdirectory(common)
add_subdirectory(cashflow)
add_subdirectory(loan)
add_subdirectory(tax)
add_subdirectory(transactions)
add_subdirectory(simulation)

add_subdirectory(json_interface)


#############################################
# Install and export
#############################################
#gather headers list
file(GLOB_RECURSE HeadersList
  "*.h"
  "*.hpp"
)

install( FILES
  ${HeadersList}
  DESTINATION include/fisim
)

#install(FILES fisim-config.cmake DESTINATION lib/cmake/fisim)

install(EXPORT export_set
  FILE
    fisim-config.cmake
  DESTINATION
    lib/cmake/fisim
)

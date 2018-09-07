# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(SimulationValidationModule)
find_package(Falaise REQUIRED)

# Build a dynamic library from our sources
add_library(SimulationValidationModule SHARED SimulationValidationModule.h SimulationValidationModule.cpp)

# Link it to the FalaiseModule library
# This ensures the correct compiler flags, include paths
# and linker flags are applied to our dynamic library.
# - Explicit link to ROOT Physics library which is not linked
#   by flreconstruct
target_link_libraries(SimulationValidationModule
  PUBLIC
    Falaise::FalaiseModule
    ${ROOT_Physics_LIBRARY}
    )

# Configure example pipeline script for use from the build dir
configure_file("SimulationValidationModuleExample.conf.in" "SimulationValidationModuleExample.conf" @ONLY)

# Add a basic test of reading a brio file output by the
# standard pipeline
enable_testing()
# - Simulate
add_test(NAME testSimulationValidationModule_simulate
  COMMAND Falaise::flsimulate -o test-simulate.brio
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
# - Reconstruct
add_test(NAME testSimulationValidationModule_reconstruct
  COMMAND Falaise::flreconstruct -i test-simulate.brio -p urn:snemo:demonstrator:reconstruction:2.0.0 -o test-reconstruct.brio
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
set_tests_properties(testSimulationValidationModule_reconstruct
  PROPERTIES DEPENDS testSimulationValidationModule_simulate
  )
# - Run Module
add_test(NAME testSimulationValidationModule_Validation
  COMMAND Falaise::flreconstruct -i test-reconstruct.brio -p SimulationValidationModuleExample.conf
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
set_tests_properties(testSimulationValidationModule_Validation
  PROPERTIES DEPENDS testSimulationValidationModule_reconstruct
  )


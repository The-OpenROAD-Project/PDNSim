cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For Multithread
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -O3")

set(CMAKE_EXE_LINKER_FLAGS "-ltcl")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(PDNSIM_HOME ${PROJECT_SOURCE_DIR}/src/PDNSim)
set(OPENSTA_HOME ${PROJECT_SOURCE_DIR}/src/OpenSTA)

set(SUPERLU_HOME ${PDNSIM_HOME}/modules/superlu)

set(OPENROAD_HOME ${PROJECT_SOURCE_DIR})
set(OPENDB_HOME ${PROJECT_SOURCE_DIR}/src/OpenDB)

set(PDNSIM_WRAP ${CMAKE_CURRENT_BINARY_DIR}/pdnsim_wrap.cpp)

## configure settings
find_package(TCL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SWIG REQUIRED)

set (PDNSIM_SRC 
  node.cpp
  gmat.cpp
  ir_solver.cpp
  get_power.cpp
  main.cpp
  MakePDNSim.cpp
  pdnsim.cpp
  ${PDNSIM_WRAP}
  )


set (PDNSIM_HEADER
  ir_solver.h
  node.h
  gmat.h
  get_power.h
  ${PDNSIM_HOME}/include/MakePDNSim.hh
  ${PDNSIM_HOME}/include/pdnsim.h
)

add_custom_command(OUTPUT ${PDNSIM_WRAP}
  COMMAND ${SWIG_EXECUTABLE} -tcl8 -c++ -o ${PDNSIM_WRAP} ${PDNSIM_HOME}/src/pdnsim.i 
  WORKING_DIRECTORY ${PDNSIM_HOME}
  DEPENDS ${PDNSIM_HOME}/src/pdnsim.i ${PDNSIM_HOME}/include/pdnsim.h
)

############################################################
# Create a library 
############################################################
add_library( pdnsim 
    ${PDNSIM_SRC}
)

############################################################
# include directories 
############################################################
target_include_directories( pdnsim
PUBLIC
  ${PDNSIM_HOME}/include
PRIVATE
  ${PDNSIM_HOME}/src

  ${SUPERLU_HOME}/SRC
  ${SUPERLU_HOME}/CBLAS

  ${OPENSTA_HOME}/util
  ${OPENSTA_HOME}/liberty
  ${OPENSTA_HOME}/network
  ${OPENSTA_HOME}/graph
  ${OPENSTA_HOME}/sdc
  ${OPENSTA_HOME}/parasitics
  ${OPENSTA_HOME}/search
  ${OPENSTA_HOME}/app

  ${OPENDB_HOME}/include
  ${OPENROAD_HOME}/include
  ${TCL_INCLUDE_PATH}
)

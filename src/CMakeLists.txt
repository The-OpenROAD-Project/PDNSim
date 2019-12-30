 
set(IRSOLVER_HOME ${PROJECT_SOURCE_DIR})
set(OPENDB_HOME ${PROJECT_SOURCE_DIR}/src/OpenDB)
message(STATUS "TCL lib: ${TCL_LIB}")

################################################################
#
# Library dependencies
#
################################################################

find_package(TCL)
message(STATUS "TCL lib: ${TCL_LIB}")
message(STATUS "TCL header: ${TCL_HEADER}")


set(BUILD_PYTHON false)
set(BUILD_TCL true)
add_subdirectory(OpenDB)


set(IRSOLVER_SOURCE IRSolver.cpp)


add_executable(irsolver
  ${IRSOLVER_SOURCE})
target_include_directories(irsolver
  ${IRSOLVER_HOME}/include
  ${IRSOLVER_HOME}/src
  ${OPENDB_HOME}/include/opendb
  ${OPENDB_HOME}/src/db
  )



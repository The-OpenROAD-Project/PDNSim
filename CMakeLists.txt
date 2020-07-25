cmake_minimum_required(VERSION 3.9)

project(PDNSIM VERSION 0.1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable compiler specific extensions like gnu++11.
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -O3")


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(PDNSIM_HOME ${PROJECT_SOURCE_DIR} )

set(OPENDB_HOME
    ${PROJECT_SOURCE_DIR}/modules/OpenDB
  )
set(OPENSTA_HOME
${PROJECT_SOURCE_DIR}/modules/OpenSTA
)
set(SUPERLU_HOME
${PROJECT_SOURCE_DIR}/modules/superlu
)


set(BUILD_PYTHON OFF)
set(BUILD_TCL ON)

add_subdirectory(${OPENDB_HOME} ${CMAKE_CURRENT_BINARY_DIR}/module/OpenDB)
add_subdirectory(${OPENSTA_HOME})
add_subdirectory(${SUPERLU_HOME})

set(ENV{TYPE_CALL} library)


## configure settings
find_package(TCL REQUIRED)

find_package(ZLIB REQUIRED)
find_package(SWIG REQUIRED)


# for cimg
#if( USE_CIMG_LIB )
#  find_package(X11 REQUIRED)
#  find_package(JPEG REQUIRED)
#  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${X11_LIBRARIES} ${JPEG_LIBRARIES}")
#else()
#  find_package(X11)
#  find_package(JPEG)
#endif()


set (PDNSIM_SRC 
  src/pdnsim_external.cpp
  src/node.cpp
  src/gmat.cpp
  src/ir_solver.cpp
  src/get_power.cpp
  src/main.cpp
  src/pdnsim_wrap.cpp
  )


set (PDNSIM_HEADER
  src/ir_solver.h
  src/node.h
  src/gmat.h
  src/pdnsim_external.h
  src/get_power.h
)

set (PDNSIM_SWIG_FILES 
  ${PDNSIM_HOME}/src/ir_solver.i
)

  
############################################################
# Create a library 
############################################################
add_library( pdnsimlib  SHARED
        ${PDNSIM_SRC}
)

set_target_properties( pdnsimlib 
  PROPERTIES OUTPUT_NAME "pdnsim"
)

############################################################
# Create an executable
############################################################
add_custom_command(OUTPUT ${PDNSIM_HOME}/src/pdnsim_wrap.cpp
        COMMAND ${SWIG_EXECUTABLE} -tcl8 -c++ -o
        ${PDNSIM_HOME}/src/pdnsim_wrap.cpp  ${PDNSIM_HOME}/src/ir_solver.i 
  WORKING_DIRECTORY ${PDNSIM_HOME}
  DEPENDS ${PDNSIM_SWIG_FILES}
)



add_executable( pdnsim
  ${PDNSIM_SRC}
)  

add_dependencies( pdnsim opendb )
add_dependencies( pdnsim superlu )
add_dependencies( pdnsim sta )


############################################################
# include directories 
############################################################
target_include_directories( pdnsim
  PUBLIC
  ${OPENDB_HOME}/include/opendb
  ${SUPERLU_HOME}/SRC
  ${SUPERLU_HOME}/CBLAS

  ${OPENSTA_HOME}/app
  ${OPENSTA_HOME}/dcalc
  ${OPENSTA_HOME}/graph
  ${OPENSTA_HOME}/liberty
  ${OPENSTA_HOME}/network
  ${OPENSTA_HOME}/parasitics
  ${OPENSTA_HOME}/sdc
  ${OPENSTA_HOME}/sdf
  ${OPENSTA_HOME}/search
  ${OPENSTA_HOME}/tcl
  ${OPENSTA_HOME}/util
  ${OPENSTA_HOME}/verilog
)

target_include_directories( pdnsimlib 
  PUBLIC
  ${OPENDB_HOME}/include/opendb
  ${SUPERLU_HOME}/SRC
  ${SUPERLU_HOME}/CBLAS

  ${OPENSTA_HOME}/app
  ${OPENSTA_HOME}/dcalc
  ${OPENSTA_HOME}/graph
  ${OPENSTA_HOME}/liberty
  ${OPENSTA_HOME}/network
  ${OPENSTA_HOME}/parasitics
  ${OPENSTA_HOME}/sdc
  ${OPENSTA_HOME}/sdf
  ${OPENSTA_HOME}/search
  ${OPENSTA_HOME}/tcl
  ${OPENSTA_HOME}/util
  ${OPENSTA_HOME}/verilog
)

target_link_libraries(pdnsim
  PUBLIC
  ${TCL_LIBRARY}
  OpenSTA
  ${ZLIB_LIBRARIES}
  superlu
  opendb
  )

############################################################
# Install  
############################################################

install(FILES ${PDNSIM_HEADER} DESTINATION include)
install(TARGETS pdnsim DESTINATION bin)
install(TARGETS pdnsimlib DESTINATION lib)


cmake_minimum_required(VERSION 3.9)

project(IRSolver VERSION 0.1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable compiler specific extensions like gnu++11.
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_CIMG_LIB "Use CImg drawing library for image drawing" OFF)
if( USE_CIMG_LIB )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_jpeg=1 -O3 -D_GLIBCXX_USE_CXX11_ABI=0")
  set(CIMG_HOME ${PROJECT_SOURCE_DIR}/module/CImg)
else()
  add_definitions(-DDISABLE_CIMG_LIB=1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -D_GLIBCXX_USE_CXX11_ABI=0")
endif()

# For Multithread
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_jpeg=1 -DUSE_FFT2D_PTHREADS=1 -DFFT_2D_MAX_THREADS=16 -O3 ${OpenMP_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(IRSOLVER_HOME ${PROJECT_SOURCE_DIR} )

set(BUILD_PYTHON OFF)
set(BUILD_TCL OFF)

set(OPENDB_HOME
    ${PROJECT_SOURCE_DIR}/src/OpenDB
  )
add_subdirectory(${OPENDB_HOME})

set(ENV{TYPE_CALL} library)


## configure settings
find_package(TCL REQUIRED)


# for cimg
if( USE_CIMG_LIB )
  find_package(X11 REQUIRED)
  find_package(JPEG REQUIRED)
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${X11_LIBRARIES} ${JPEG_LIBRARIES}")
else()
  find_package(X11)
  find_package(JPEG)
endif()


find_package(Boost REQUIRED)
#find_package(Threads REQUIRED)

# for multi threads
find_package(OpenMP)

set (IRSOLVER_SRC 
  src/IRSolver.cpp
  )


set (IRSOLVER_HEADER
  src/IRSolver.hh
)

set (IRSOLVER_SWIG_FILES
)

  
############################################################
# Create a library 
############################################################
add_library( irsolverlib  SHARED
  ${IRSOLVER_SRC}
)

set_target_properties( irsolverlib 
  PROPERTIES OUTPUT_NAME "irsolver"
)

############################################################
# Create an executable
############################################################

add_executable( irsolver
  ${IRSOLVER_SRC}
)  

add_dependencies( irsolver opendb )

add_dependencies( irsolverlib opendb )

############################################################
# include directories 
############################################################
target_include_directories( irsolver
  PUBLIC
  ${OPENDB_HOME}/include/opendb
)

target_include_directories( irsolverlib 
  PUBLIC
  ${OPENDB_HOME}/include/opendb
)

target_link_libraries( irsolver
  PUBLIC
  ${TCL_LIB}
  opendb
  )

############################################################
# Install  
############################################################

install(FILES ${IRSOLVER_HEADER} DESTINATION include)
install(TARGETS irsolver DESTINATION bin)
install(TARGETS irsolverlib DESTINATION lib)


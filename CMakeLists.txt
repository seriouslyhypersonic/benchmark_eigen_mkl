cmake_minimum_required(VERSION 3.13)
project(benchmark_eigen_mkl)

set(CMAKE_CXX_STANDARD 17)

# ------------------------------------------------------------------------------
# Add Eigen
# ------------------------------------------------------------------------------
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(OpenMP REQUIRED)

# ------------------------------------------------------------------------------
# Intel MKL
# ------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(MKL)
if (MKL_FOUND)
    message(STATUS "Search summary")
    message(STATUS "    MKL interface library        ${MKL_INTERFACE_LIBRARY}")
    message(STATUS "    MKL core library             ${MKL_CORE_LIBRARY}")
    message(STATUS "    MKL threading layer library  ${MKL_THREADING_LAYER_LIBRARY}")
    message(STATUS "    MKL include                  ${MKL_INCLUDE_DIRS}")
    message(STATUS "    MKL libraries                ${MKL_LIBRARIES}")
endif ()

# ------------------------------------------------------------------------------
# Add Celero as an external project
# ------------------------------------------------------------------------------

find_package(Git REQUIRED)
include(ExternalProject)

message(STATUS "Configuring Celero")
ExternalProject_Add(celero_project
        GIT_REPOSITORY https://github.com/DigitalInBlue/Celero.git
        GIT_TAG master
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        INSTALL_DIR ${CMAKE_BINARY_DIR}
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DCELERO_COMPILE_DYNAMIC_LIBRARIES:BOOL=OFF
        -DCELERO_ENABLE_EXPERIMENTS:BOOL=OFF
        -DCELERO_ENABLE_FOLDERS:BOOL=OFF)
ExternalProject_Get_Property(celero_project SOURCE_DIR)
ExternalProject_Get_Property(celero_project INSTALL_DIR)
set(CELERO_INSTALL_DIR ${INSTALL_DIR})
message(STATUS "CELERO_INSTALL_DIR=${CELERO_INSTALL_DIR}")

include_directories(${CELERO_INSTALL_DIR}/include)
add_library(libcelero STATIC IMPORTED GLOBAL)
set_property(TARGET libcelero PROPERTY
        IMPORTED_LOCATION_DEBUG
        "${CELERO_INSTALL_DIR}/lib/static/${CMAKE_STATIC_LIBRARY_PREFIX}celerod${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(TARGET libcelero PROPERTY
        IMPORTED_LOCATION_RELEASE
        "${CELERO_INSTALL_DIR}/lib/static/${CMAKE_STATIC_LIBRARY_PREFIX}celero${CMAKE_STATIC_LIBRARY_SUFFIX}")
add_dependencies(libcelero celero_project)

# ------------------------------------------------------------------------------
# Benchmark
# ------------------------------------------------------------------------------

# --- Print debug information
option(OPTION_PRINT_DEBUG_INFORMATION "Prints debug information" OFF)

set(HEADERS include/debug.hpp include/mkl_utils.hpp include/random_vector.hpp include/fixture.hpp)
set(SOURCES_TESTS test/test.cpp)

add_executable(benchmark_dgemm ${HEADERS} benchmark_dgemm.cpp)
add_executable(benchmark_madd ${HEADERS} benchmark_madd.cpp)

add_executable(tests_run ${HEADERS} ${SOURCES_TESTS})

target_include_directories(benchmark_dgemm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${MKL_INCLUDE_DIRS})

target_include_directories(benchmark_madd PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${MKL_INCLUDE_DIRS})

target_include_directories(tests_run PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${MKL_INCLUDE_DIRS})

target_link_libraries(benchmark_dgemm
        libcelero
        Eigen3::Eigen OpenMP::OpenMP_CXX
        ${MKL_LIBRARIES})

target_link_libraries(benchmark_madd
        libcelero
        Eigen3::Eigen OpenMP::OpenMP_CXX
        ${MKL_LIBRARIES})

target_link_libraries(tests_run
        libcelero
        Eigen3::Eigen OpenMP::OpenMP_CXX
        ${MKL_LIBRARIES})

if (OPTION_PRINT_DEBUG_INFORMATION)
    target_compile_definitions(benchmark_dgemm PRIVATE PRINT_DEBUG_INFORMATION)
    target_compile_definitions(benchmark_madd PRIVATE PRINT_DEBUG_INFORMATION)
    target_compile_definitions(tests_run PRIVATE PRINT_DEBUG_INFORMATION)
    message(STATUS "Debug information will be printed to stdout")
endif ()

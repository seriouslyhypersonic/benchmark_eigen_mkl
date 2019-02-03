# Copyright (c) Nuno Alves de Sousa 2019
#
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
# Find Intel(R) Math Kernel Library (64 bit)
#
# Useful references:
#     https://software.intel.com/en-us/articles/intel-mkl-link-line-advisor
#     https://software.intel.com/en-us/mkl-linux-developer-guide-linking-with-threading-libraries
#
# To find, include and link with MKL:
# MKL_FOUND                   - System has MKL
# MKL_INCLUDE_DIRS            - MKL include directories
# MKL_LIBRARIES               - The MKL libraries and link advisor dependecies
#
# For optional detection summary:
# MKL_INTERFACE_LIBRARY       - MKL interface library
# MKL_CORE_LIBRARY            - MKL core library
# MKL_THREADING_LAYER_LIBRARY - MKL core library
#
# Requires one of the following envirnment variables:
#     - MKLROOT points to the MKL root
#     - INTEL   points to the parent directory of MKLROOT
#
# Execution mode is selected with -DMKL_THREADING=<intel | gnu | sequential>
#     - intel      uses Intel(R) OpenMP libraries
#     - gnu        uses GNU OpenMP libraries
#     - sequential no parallel execution
#
# Basic usage:
# find_package(MKL)
# if(MKL_FOUND)
#     target_include_directories(TARGET PRIOVATE ${MKL_INCLUDE_DIRS})
#     target_link_libraries(TARGET ${MKL_LIBRARIES})
# endif()

message(STATUS "Check for Intel(R) Math Kernel libraries")

# If already in cache, be silent
#if (MKL_INCLUDE_DIRS
#        AND MKL_LIBRARIES
#        AND MKL_INTERFACE_LIBRARY
#        AND MKL_CORE_LIBRARY
#        AND MKL_THREADING_LAYER_LIBRARY)
#    set (MKL_FIND_QUIETLY TRUE)
#endif()

# --- Detect execution mode
if (MKL_THREADING STREQUAL "intel")
    message(STATUS "MKL execution mode: parallel (Intel(R) OpenMP library)")
    set(MKL_THREADING_INTEL TRUE)
elseif (MKL_THREADING STREQUAL "gnu")
    if (APPLE)
        message(FATAL_ERROR
                "On MacOS X, the GNU threading layer is unsupported.\n"
                "You can select Intel(R) instead")
    endif ()

    message(STATUS "MKL execution mode: parallel (GNU OpenMP library)")
    set(MKL_THREADING_GNU TRUE)
elseif (MKL_THREADING STREQUAL "sequential")
    message(STATUS "MKL execution mode: sequential")
    set(MKL_THREADING_SEQUENTIAL TRUE)
elseif(MKL_THREADING)
    message(FATAL_ERROR
            "invalid usage -DMKL_THREADING=${MKL_THREADING}\n"
            "usage -DMKL_THREADING=< intel | gnu | sequential >")
else()
    message(WARNING "No MKL_THREADING option selected. Using default (sequential)\n"
            "usage -DMKL_THREADING=< intel | gnu | sequential >")
endif()

# --- Generate appropriate filenames for MKL libraries
# Static linkage: (prefix)libname(sufix)
if(NOT BUILD_SHARED_LIBS)
    # MKL Interface layer library - mkl_intel_ilp64 (always 64 bit integer)
    set(INT_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}")
    set(INT_LIB "${INT_LIB}mkl_intel_ilp64")
    set(INT_LIB "${INT_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    # MKL Core library - mkl_core
    set(COR_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}")
    set(COR_LIB "${COR_LIB}mkl_core")
    set(COR_LIB "${COR_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    # MKL threading layer library
    set(THR_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}")
    if (MKL_THREADING_INTEL)
        # use - mkl_intel_thread
        set(THR_LIB "${THR_LIB}mkl_intel_thread")
    elseif (MKL_THREADING_GNU)
        # use - mkl_gnu_thread
        set(THR_LIB "${THR_LIB}mkl_gnu_thread")
    else()
        # use - mkl_sequential
        set(THR_LIB "${THR_LIB}mkl_sequential")
    endif()
    set(THR_LIB "${THR_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    # TODO: handle this in the future noting that on windows libname_dll.dll
    set(INT_LIB "mkl_intel_ilp64.")
    #set(SEQ_LIB "mkl_sequential")
    set(THR_LIB "mkl_intel_thread")
    set(COR_LIB "mkl_core")
endif()

# --- Generate appropriate filename for run-time threading library
set(RTL_LIB "${CMAKE_SHARED_LIBRARY_PREFIX}")
if (MKL_THREADING_INTEL)
    set(RTL_LIB "${RTL_LIB}iomp5")
elseif(MKL_THREADING_GNU)
    set(RTL_LIB "${RTL_LIB}gomp")
endif()
set(RTL_LIB "${RTL_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}")

if (MKL_THREADING_SEQUENTIAL)
    set(RTL_LIB "")
endif ()

# --- Find MKL summary
message(STATUS "Looking for")
message(STATUS "    MKL interface library        ${INT_LIB}")
message(STATUS "    MKL core library             ${COR_LIB}")
message(STATUS "    MKL threading layer library  ${THR_LIB}")
message(STATUS "    Run-time threading library   ${RTL_LIB}")

# --- Search libraries and includes
find_library(MKL_INTERFACE_LIBRARY NAMES ${INT_LIB}
        PATHS $ENV{MKLROOT}/lib/
              $ENV{MKLROOT}/lib/intel64
              $ENV{INTEL}/mkl/lib/intel64
        NO_DEFAULT_PATH)
if(NOT MKL_INTERFACE_LIBRARY)
    message($ENV{MKLROOT})
    message(FATAL_ERROR "Cannot find MKL interface library: ${INT_LIB}")
endif()

find_library(MKL_CORE_LIBRARY  NAMES ${COR_LIB}
        PATHS $ENV{MKLROOT}/lib/
              $ENV{MKLROOT}/lib/intel64
              $ENV{INTEL}/mkl/lib/intel64
        NO_DEFAULT_PATH)
if(NOT MKL_CORE_LIBRARY)
    message(FATAL_ERROR "Cannot find MKL core library: ${COR_LIB}")
endif()

find_library(MKL_THREADING_LAYER_LIBRARY
        NAMES ${THR_LIB}
        PATHS $ENV{MKLROOT}/lib/
              $ENV{MKLROOT}/lib/intel64
              $ENV{INTEL}/mkl/lib/intel64
        NO_DEFAULT_PATH)
if(NOT MKL_THREADING_LAYER_LIBRARY)
    message(FATAL_ERROR "Cannot find MKL threading layer library: ${THR_LIB}")
endif()

find_path(MKL_INCLUDE_DIR NAMES mkl.h
        PATHS $ENV{MKLROOT}/include
              $ENV{INTEL}/mkl/include
        NO_DEFAULT_PATH)
if(NOT MKL_INCLUDE_DIR)
    message(FATAL_ERROR "Cannot find MKL include directory")
endif()

# --- Search was successful
set(MKL_LIBRARIES ${MKL_INTERFACE_LIBRARY}
        ${MKL_THREADING_LAYER_LIBRARY}
        ${MKL_CORE_LIBRARY})
set(MKL_INCLUDE_DIRS ${MKL_INCLUDE_DIR})

# --- Generate links to MKL libraries and its dependencies
# todo: other platforms
# GNU
if (UNIX)
    # Always required
    set(MKL_LINK_OPTIONS -lpthread -lm -ldl)

    # Choose required RTL
    if(MKL_THREADING_INTEL)
        set(MKL_LINK_OPTIONS ${MKL_LINK_OPTIONS} -liomp5)
    elseif(MKL_THREADING_GNU)
        set(MKL_LINK_OPTIONS ${MKL_LINK_OPTIONS} -lgomp)
    endif()

    # todo: dynamic MKL

    if (NOT APPLE) # and static
        # --start-group --end-group required
        set(MKL_LIBRARIES
                "-Wl,--start-group"  ${MKL_LIBRARIES}  "-Wl,--end-group")
    endif ()

    set(MKL_LIBRARIES ${MKL_LIBRARIES} ${MKL_LINK_OPTIONS})
endif ()

# --- Set compiler options
if (MKL_INCLUDE_DIR AND
        MKL_INTERFACE_LIBRARY AND
        MKL_CORE_LIBRARY AND
        MKL_THREADING_LAYER_LIBRARY)

    if (NOT DEFINED ENV{CRAY_PRGENVPGI}
            AND NOT DEFINED ENV{CRAY_PRGENVGNU}
            AND NOT DEFINED ENV{CRAY_PRGENVCRAY}
            AND NOT DEFINED ENV{CRAY_PRGENVINTEL})
      set(ABI "-m64")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMKL_ILP64 ${ABI}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMKL_ILP64 ${ABI}")
else()
    set(MKL_LIBRARIES "")
    set(MKL_INTERFACE_LIBRARY "")
    set(MKL_SEQUENTIAL_LAYER_LIBRARY "")
    set(MKL_CORE_LIBRARY "")
    set(MKL_INCLUDE_DIRS "")
endif()

# Handle the QUIETLY and REQUIRED arguments and set MKL_FOUND to TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MKL DEFAULT_MSG
        MKL_LIBRARIES
        MKL_INTERFACE_LIBRARY
        MKL_CORE_LIBRARY
        MKL_THREADING_LAYER_LIBRARY
        MKL_INCLUDE_DIRS)

mark_as_advanced(MKL_LIBRARIES
        MKL_INTERFACE_LIBRARY
        MKL_CORE_LIBRARY
        MKL_THREADING_LAYER_LIBRARY
        MKL_INCLUDE_DIRS)

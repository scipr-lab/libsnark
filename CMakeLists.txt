cmake_minimum_required(VERSION 2.8)

project (libsnark)

set(
  CURVE
  "BN128"
)

option(
  DEBUG
  "Enable debugging mode"
  OFF
)
option(
  LOWMEM
  "Limit the size of multi-exponentiation tables, for low-memory platforms"
  OFF
)
option(
  MULTICORE
  "Enable parallelized execution, using OpenMP"
  OFF
)
option(
  USE_PT_COMPRESSION
  "Use point compression"
  ON
)
option(
  PROFILE_OP_COUNTS
  "Collect counts for field and curve operations"
  OFF
)
option(
  USE_MIXED_ADDITION
  "Convert each element of the key pair to affine coordinates"
  OFF
)

option(
  WITH_SUPERCOP
  "Support for Ed25519 signatures required by ADSNARK"
  ON
)

option(
  WITH_PROCPS
  "Use procps for memory profiling"
  ON
)

option(
  CPPDEBUG
  "Enable debugging of C++ STL (does not imply DEBUG)"
  OFF
)

option(
  PERFORMANCE,
  "Enable link-time and aggressive optimizations"
  OFF
)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Tuning and common compilation flags
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -ggdb3 -std=c++11 -O2 -march=native -mtune=native"
  )
  # Warning configuration
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wfatal-errors"
  )
  if("${MULTICORE}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  endif()
endif()

find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp)
find_library(GMPXX_LIBRARIES NAMES gmpxx libgmpxx)

include(FindPkgConfig)
pkg_check_modules(
  CRYPTO
  REQUIRED

  libcrypto
)

if("${WITH_PROCPS}")
  pkg_check_modules(
    PROCPS
    REQUIRED

    libprocps
  )
else()
  add_definitions(
    -DNO_PROCPS
  )
endif()

# Enable Boost for program_options
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# Default feature flags.
add_definitions(
  -DUSE_ASM
  -DMONTGOMERY_OUTPUT
)

add_definitions(
  -DCURVE_${CURVE}
)

enable_testing()

include_directories(.)

if(${CURVE} STREQUAL "BN128")
  add_definitions(
    -DBN_SUPPORT_SNARK=1
  )
endif()

if("${DEBUG}")
  add_definitions(-DDEBUG=1)
endif()

if("${LOWMEM}")
  add_definitions(-DLOWMEM=1)
endif()

if("${MULTICORE}")
  add_definitions(-DMULTICORE=1)
endif()

if(NOT "${USE_PT_COMPRESSION}")
  add_definitions(-DNO_PT_COMPRESSION=1)
endif()

if("${PROFILE_OP_COUNTS}")
  add_definitions(-DPROFILE_OP_COUNTS=1)
endif()

if("${USE_MIXED_ADDITION}")
  add_definitions(-DUSE_MIXED_ADDITION=1)
endif()

if("${CPPDEBUG}")
  add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC)
endif()

if("${PERFORMANCE}")
  add_definitions(-DNDEBUG)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -flto -fuse-linker-plugin"
  )
  set(
    CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -flto"
  )
endif()

find_program(
  MARKDOWN

  markdown_py
  DOC "Path to markdown_py binary"
)
if(MARKDOWN-NOTFOUND)
else()
   add_custom_target(
     doc
     ${MARKDOWN} -f ${CMAKE_CURRENT_BINARY_DIR}/README.html -x toc -x extra --noisy ${CMAKE_CURRENT_SOURCE_DIR}/README.md
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMENT "Translating from markdown to HTML" VERBATIM
   )
endif()

add_subdirectory(third_party)
add_subdirectory(src)

cmake_minimum_required(VERSION 3.25)
project(juego_zombie LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ZS_BUILD_TESTS "Build the simulation tests" ON)

# Strict floating point everywhere: same binary, same result; see docs/tier0/specs/01-quantity.md.
add_library(zs_strict_fp INTERFACE)
if(MSVC)
  target_compile_options(zs_strict_fp INTERFACE /fp:precise /W4 /permissive- /utf-8)
else()
  target_compile_options(zs_strict_fp INTERFACE -ffp-contract=off -fno-fast-math -Wall -Wextra -Wpedantic)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i.86")
    target_compile_options(zs_strict_fp INTERFACE -msse2 -mfpmath=sse)
  endif()
endif()

add_subdirectory(engine/sim)
add_subdirectory(apps/headless)

if(ZS_BUILD_TESTS)
  enable_testing()
  set(RC_ENABLE_DOCTEST ON CACHE BOOL "" FORCE)
  add_subdirectory(third_party/rapidcheck EXCLUDE_FROM_ALL)
  add_subdirectory(engine/sim/tests)
endif()

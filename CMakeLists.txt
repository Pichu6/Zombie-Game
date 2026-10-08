cmake_minimum_required(VERSION 3.25)
project(juego_zombie LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ZS_BUILD_TESTS "Build the simulation tests" ON)
# The window fetches SDL3 and Dear ImGui; on by default on Windows (the user's PC), off elsewhere.
# Off, zs still builds with its headless modes and generators.
option(ZS_BUILD_WINDOW "Build zs with its window (spec 16)" ${WIN32})

# Strict floating point everywhere: same binary, same result; see docs/tier0/specs/01-quantity.md.
add_library(zs_strict_fp INTERFACE)
if(MSVC)
  target_compile_options(zs_strict_fp INTERFACE /fp:precise /W4 /permissive- /utf-8)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # clang-cl takes MSVC's flags but keeps contraction (FMA) on without this; contraction breaks 7 goldens (DET-16).
    target_compile_options(zs_strict_fp INTERFACE /clang:-ffp-contract=off)
  elseif(MSVC_VERSION LESS 1930)
    message(FATAL_ERROR "MSVC 2022 or newer is required (spec 01 rule 4)")
  endif()
else()
  target_compile_options(zs_strict_fp INTERFACE -ffp-contract=off -fno-fast-math -Wall -Wextra -Wpedantic)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i.86")
    target_compile_options(zs_strict_fp INTERFACE -msse2 -mfpmath=sse)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
    # One number parser: toml++ reads floats with from_chars, as it does on MSVC (DET-38).
    target_compile_definitions(zs_strict_fp INTERFACE TOML_FLOAT_CHARCONV=1)
  endif()
endif()

add_subdirectory(engine/sim)
add_subdirectory(engine/gen)
add_subdirectory(apps/zs)

if(ZS_BUILD_TESTS)
  enable_testing()
  set(RC_ENABLE_DOCTEST ON CACHE BOOL "" FORCE)
  add_subdirectory(third_party/rapidcheck EXCLUDE_FROM_ALL)
  add_subdirectory(engine/sim/tests)
  add_subdirectory(engine/gen/tests)
endif()

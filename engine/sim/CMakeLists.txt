add_library(zs_sim
  src/detmath.cpp
  src/philox.cpp
  src/draw.cpp
  src/ledger.cpp
  src/hazard.cpp
  src/events.cpp
  src/clock.cpp
  src/archive.cpp
  src/world.cpp
  src/demo.cpp
  src/assembly.cpp
  src/site_map.cpp
  src/body.cpp
  src/body_lab.cpp
  src/actions.cpp
  src/tier0.cpp
  src/power.cpp
  src/noise.cpp
  src/census.cpp
  src/walkers.cpp
  src/pop_ai.cpp
  src/run.cpp
  src/pop.cpp
  src/pop_body.cpp
  src/pop_registry.cpp
  src/demography.cpp
  src/roster.cpp
  src/pop_params.cpp
  src/pop_system.cpp
  src/hypergeometric.cpp
  src/demographics.cpp
  src/economy.cpp
)
target_include_directories(zs_sim PUBLIC include)
target_include_directories(zs_sim PRIVATE ${PROJECT_SOURCE_DIR}/third_party/tomlplusplus)
target_compile_definitions(zs_sim PUBLIC ZS_DATA_DIR="${PROJECT_SOURCE_DIR}/data")
target_link_libraries(zs_sim PUBLIC zs_strict_fp)

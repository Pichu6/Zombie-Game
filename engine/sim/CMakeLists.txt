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
)
target_include_directories(zs_sim PUBLIC include)
target_link_libraries(zs_sim PUBLIC zs_strict_fp)

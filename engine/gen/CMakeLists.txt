# Content generators (docs/design/content-tools.md): names, pops, towns. Headless, deterministic.
add_library(zs_gen
  src/catalog.cpp
  src/names.cpp
  src/pops.cpp
)
target_include_directories(zs_gen PUBLIC include)
target_include_directories(zs_gen PRIVATE ${PROJECT_SOURCE_DIR}/third_party/tomlplusplus)
target_link_libraries(zs_gen PUBLIC zs_sim)


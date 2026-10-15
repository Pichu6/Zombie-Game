set(ZS_ZS_GEN_TESTS_SOURCES
  test_gen.cpp
  test_registry_bridge.cpp
  test_world_grid.cpp
  test_world_fields.cpp
  test_place.cpp
  test_country_town.cpp
  test_world_people.cpp
  test_dwellings.cpp
  test_stations.cpp
  test_town_layout.cpp
  test_plot_contents.cpp
  test_world_land.cpp
  test_world_deposits.cpp
  test_world_countries.cpp
  test_world_routes.cpp
  test_town_travel.cpp
  test_world_food.cpp
  test_town_food.cpp
  test_world_readable.cpp
  test_world_setup.cpp
  test_people_placement.cpp
  test_land_use.cpp
  test_continents.cpp)
add_executable(zs_gen_tests main.cpp ${ZS_ZS_GEN_TESTS_SOURCES})
target_include_directories(zs_gen_tests PRIVATE ${PROJECT_SOURCE_DIR}/third_party/doctest)
target_link_libraries(zs_gen_tests PRIVATE zs_gen)
# One ctest entry per test file, so ctest runs them in parallel (testPresets jobs): a full run takes the time of the
# slowest file, not the sum. doctest's --source-file filter picks each file's test cases.
foreach(src ${ZS_ZS_GEN_TESTS_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_test(NAME zs_gen_tests.${name} COMMAND zs_gen_tests --source-file=*${src})
endforeach()

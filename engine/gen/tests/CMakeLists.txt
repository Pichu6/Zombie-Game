add_executable(zs_gen_tests main.cpp test_gen.cpp test_registry_bridge.cpp test_world_grid.cpp test_world_fields.cpp test_place.cpp test_country_town.cpp test_world_people.cpp test_dwellings.cpp test_stations.cpp test_town_layout.cpp test_plot_contents.cpp test_world_land.cpp test_world_deposits.cpp test_world_countries.cpp test_world_routes.cpp test_town_travel.cpp test_world_food.cpp)
target_include_directories(zs_gen_tests PRIVATE ${PROJECT_SOURCE_DIR}/third_party/doctest)
target_link_libraries(zs_gen_tests PRIVATE zs_gen)
add_test(NAME zs_gen_tests COMMAND zs_gen_tests)

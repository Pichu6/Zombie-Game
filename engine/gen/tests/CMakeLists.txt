add_executable(zs_gen_tests main.cpp test_gen.cpp test_registry_bridge.cpp test_world_grid.cpp test_world_fields.cpp)
target_include_directories(zs_gen_tests PRIVATE ${PROJECT_SOURCE_DIR}/third_party/doctest)
target_link_libraries(zs_gen_tests PRIVATE zs_gen)
add_test(NAME zs_gen_tests COMMAND zs_gen_tests)

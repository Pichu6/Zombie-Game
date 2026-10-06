add_executable(zs_sim_tests
  main.cpp
  test_quantity.cpp
  test_detmath.cpp
  test_draw.cpp
  test_ledger.cpp
  test_hazard_events.cpp
  test_clock.cpp
  test_save_replay.cpp
  test_stage2.cpp
  test_body.cpp
  test_stage4.cpp
  test_stage6.cpp
  test_stage7.cpp
  test_stage8.cpp
  test_stage9.cpp
  test_pop.cpp
  test_pop_registry.cpp
  test_demography.cpp
)
target_include_directories(zs_sim_tests PRIVATE ${PROJECT_SOURCE_DIR}/third_party/doctest)
target_link_libraries(zs_sim_tests PRIVATE zs_sim rapidcheck)
target_compile_definitions(zs_sim_tests PRIVATE ZS_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
add_test(NAME zs_sim_tests COMMAND zs_sim_tests)

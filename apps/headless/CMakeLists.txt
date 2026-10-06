add_executable(zs_headless main.cpp pop_bench.cpp town_demo.cpp)
target_link_libraries(zs_headless PRIVATE zs_sim zs_gen)
